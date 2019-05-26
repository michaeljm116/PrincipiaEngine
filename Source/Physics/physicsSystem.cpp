#include "physicsSystem.h"
#include "../Utility/resourceManager.h"
#include <glm/gtc/type_ptr.hpp>
#include "../Utility/Input.h"
#include "../pch.h"

PhysicsSystem::PhysicsSystem()
{
	addComponentType<RigidBodyComponent>();
	addComponentType<NodeComponent>();
	addComponentType<PrimitiveComponent>();
}


PhysicsSystem::~PhysicsSystem()
{
	int i;
	for (i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}


	//delete collision shapes
	for (int j = 0; j < collisionShapes.size(); j++)
	{
		btCollisionShape* shape = collisionShapes[j];
		collisionShapes[j] = 0;
		delete shape;
	}

	//delete dynamics world
	delete dynamicsWorld;

	//delete solver
	delete solver;

	//delete broadphase
	delete overlappingPairCache;

	//delete dispatcher
	delete dispatcher;

	delete collisionConfiguration;

	//next line is optional: it will be cleared by the destructor when the array goes out of scope
	collisionShapes.clear();
}

void PhysicsSystem::initialize()
{
	collisionConfiguration = new btDefaultCollisionConfiguration();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	dispatcher = new btCollisionDispatcher(collisionConfiguration);

	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	overlappingPairCache = new btDbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	solver = new btSequentialImpulseConstraintSolver;

	dynamicsWorld = new btSimpleDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0, -9.8f, 0));

	em = world->getEntityManager();
	sm = world->getSystemManager();
	rs = (RenderSystem*)sm->getSystem<RenderSystem>();
	ts = (TransformSystem*)sm->getSystem<TransformSystem>();

	nodeMapper.init(*world);
	rbMapper.init(*world);
	objMapper.init(*world);


	fake.position = glm::vec3(0.f);
	fake.rotation = glm::quat();
	fake.scale = glm::vec3(1.f);
}


void PhysicsSystem::processEntity(artemis::Entity & e)
{
	NodeComponent* nc = nodeMapper.get(e);
	RigidBodyComponent* rbc = rbMapper.get(e);

	btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[rbc->coaIndex];
	rbc->trans = obj->getWorldTransform();

	TransformComponent* tc = (TransformComponent*)nc->data->getComponent<TransformComponent>();
	rbc->trans.getOpenGLMatrix(glm::value_ptr(tc->world));
	tc->local.position = tc->world[3];
	tc->local.rotation = glm::toQuat(tc->world);

	if (nc->flags & COMPONENT_MODEL) {
		ts->processEntity(e);
	}
	else {
		ts->SQTTransform(nc, fake);
	}
}

void PhysicsSystem::addGroundForNow()
{
	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);// new btBoxShape(btVector3(btScalar(50.), btScalar(1.), btScalar(50.)));

	collisionShapes.push_back(groundShape);

	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(0, -5, 0));

	btScalar mass(0.);

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		groundShape->calculateLocalInertia(mass, localInertia);

	//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
	btRigidBody* body = new btRigidBody(rbInfo);

	//add the body to the dynamics world
	dynamicsWorld->addRigidBody(body);
}

//Types: SPHERE = -1, BOX = -2, CYLINDER = -3, PLANE = -4
void PhysicsSystem::addNode(NodeComponent * node)
{
	TransformComponent* tc = (TransformComponent*)node->data->getComponent<TransformComponent>();
	RigidBodyComponent* rbc = (RigidBodyComponent*)node->data->getComponent<RigidBodyComponent>();
	PrimitiveComponent* oc = (PrimitiveComponent*)node->data->getComponent<PrimitiveComponent>();
	//CollisionComponent* cc = (CollisionComponent*)node->data->getComponent<CollisionComponent>();
	btVector3 inertia = btVector3(0.f, 0.f, 0.f);
	btScalar mass = 1.f;
	if (!node->isDynamic) mass = 0.f;
	if (oc->uniqueID > 0) {
		//btCollisionShape* model = new btBoxShape()
		btCollisionShape* box = new btBoxShape(btVector3(tc->local.scale.x, tc->local.scale.y, tc->local.scale.z));
		//btScalar radius = sqrt((tc->local.scale.x * tc->local.scale.x) + (tc->local.scale.z * tc->local.scale.z));
		//btCollisionShape* box = new btCylinderShapeZ(btVector3(radius, tc->local.scale.y, radius));//(radius, tc->local.scale.y,);
		collisionShapes.push_back(box);
		if(node->isDynamic)
			box->calculateLocalInertia(mass, inertia);

		btDefaultMotionState* myMotionState = new btDefaultMotionState(rbc->trans);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, box, inertia);
		btRigidBody* body = new btRigidBody(rbInfo);\
		dynamicsWorld->addRigidBody(body);
		rbc->coaIndex = dynamicsWorld->getCollisionObjectArray().size() - 1;
	}
	if (oc->uniqueID  == (int)ObjectType::SPHERE) {
		btCollisionShape* sphere = new btSphereShape(btScalar(tc->local.scale.x));
		collisionShapes.push_back(sphere);
		if (node->isDynamic)
			sphere->calculateLocalInertia(mass, inertia);

		btDefaultMotionState* myMotionState = new btDefaultMotionState(rbc->trans);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, sphere, inertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		body->setRollingFriction(1.1f);
		body->setSpinningFriction(1.1f);

		dynamicsWorld->addRigidBody(body);
		rbc->coaIndex = dynamicsWorld->getCollisionObjectArray().size() - 1;
		rbc->body = body;
	}
	if (oc->uniqueID == (int)ObjectType::BOX) {
		btCollisionShape* box = new btBoxShape(btVector3(tc->local.scale.x, tc->local.scale.y, tc->local.scale.z));
		collisionShapes.push_back(box);
		if (node->isDynamic)
			box->calculateLocalInertia(mass, inertia);

		btDefaultMotionState* myMotionState = new btDefaultMotionState(rbc->trans);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, box, inertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		//btCollisionObject* body = new btCollisionObject();
		//body->setCollisionShape(box);
		//dynamicsWorld->addCollisionObject(body);
		dynamicsWorld->addRigidBody(body);
		rbc->coaIndex = dynamicsWorld->getCollisionObjectArray().size() - 1;
	}

	if (oc->uniqueID == (int)ObjectType::PLANE) {
		//btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
		btCollisionShape* plane = new btStaticPlaneShape(btVector3(tc->eulerRotation.x, tc->eulerRotation.y, tc->eulerRotation.z), 0);// new btBoxShape(btVector3(tc->eulerRotation.x, tc->eulerRotation.y, tc->eulerRotation.z));
		collisionShapes.push_back(plane);

		//btTransform groundTransform = rbc->trans;
		rbc->trans.setOrigin(btVector3(tc->local.position.x * -tc->eulerRotation.x, tc->local.position.x * -tc->eulerRotation.y, tc->local.position.x * -tc->eulerRotation.z));
		//rbc->trans = groundTransform;

		//startTransform.setOrigin(btVector3(tc->local.position.x, tc->local.position.y, tc->local.position.z));
		btDefaultMotionState* myMotionState = new btDefaultMotionState(rbc->trans);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, plane, inertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		dynamicsWorld->addRigidBody(body);
		rbc->coaIndex = dynamicsWorld->getCollisionObjectArray().size() - 1;
		//btCollisionWorld::ContactResultCallback
	}
	if (oc->uniqueID == (int)ObjectType::CYLINDER) {
		//btCollisionShape* cylinder = new btCylinderShape(btVector3(tc->local.scale.x * 0.5f, tc->local.scale.z * 0.5f, tc->local.scale.y * 0.5f));
		//btCollisionShape* cylinder = new btCylinderShapeZ()
		//btCollisionShape* cylinder = new btCapsuleShape(tc->local.scale.x * .5f, tc->local.scale.z * .5f);
		btCollisionShape* cylinder = new btBoxShape(btVector3(tc->local.scale.x * 0.5f, tc->local.scale.y * 0.5f, tc->local.scale.x * 0.5f));
		collisionShapes.push_back(cylinder);

		rbc->trans.setOrigin(btVector3(tc->local.position.x, tc->local.position.y, tc->local.position.z));

		if (node->isDynamic)
			cylinder->calculateLocalInertia(mass, inertia);

		btDefaultMotionState* myMotionState = new btDefaultMotionState(rbc->trans);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, cylinder, inertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		dynamicsWorld->addRigidBody(body);
		rbc->coaIndex = dynamicsWorld->getCollisionObjectArray().size() - 1;
	}

	//PhysicsEntity pe;
	//pe.entity = node->data;
	//pe.colObj = dynamicsWorld->getCollisionObjectArray()[rbc->coaIndex];

	//ps_Entities.push_back(pe);

	ps_Entities[dynamicsWorld->getCollisionObjectArray()[rbc->coaIndex]] = node->data->getId();
}

void PhysicsSystem::deleteNode(NodeComponent * node)
{
	RigidBodyComponent* rbc = (RigidBodyComponent*)node->data->getComponent<RigidBodyComponent>();
	//dynamicsWorld->removeCollisionObject(rbc->body);
	dynamicsWorld->removeRigidBody(rbc->body);
	
}

void PhysicsSystem::addCol(NodeComponent * node)
{
	TransformComponent* tc = (TransformComponent*)node->data->getComponent<TransformComponent>();
	RigidBodyComponent* rbc = (RigidBodyComponent*)node->data->getComponent<RigidBodyComponent>();
	PrimitiveComponent* oc = (PrimitiveComponent*)node->data->getComponent<PrimitiveComponent>();
	btVector3 inertia = btVector3(0.f, 0.f, 0.f);
	btScalar mass = 1.f;
	if (!node->isDynamic) mass = 0.f;
	if (oc->uniqueID == (int)ObjectType::BOX) {
		btCollisionShape* box = new btBoxShape(btVector3(tc->local.scale.x, tc->local.scale.y, tc->local.scale.z));
		collisionShapes.push_back(box);
		if (node->isDynamic)
			box->calculateLocalInertia(mass, inertia);

		btDefaultMotionState* myMotionState = new btDefaultMotionState(rbc->trans);
		btCollisionObject* body = new btCollisionObject();

		dynamicsWorld->addCollisionObject(body);
		rbc->coaIndex = dynamicsWorld->getCollisionObjectArray().size() - 1;
	}

}

void PhysicsSystem::update()
{
	int numManifolds = dispatcher->getNumManifolds();
	for (int i = 0; i < numManifolds; ++i) {
		btPersistentManifold* contactManifold = dispatcher->getManifoldByIndexInternal(i);
		const btCollisionObject *a = contactManifold->getBody0();
		const btCollisionObject *b = contactManifold->getBody1();
		int numContacts = contactManifold->getNumContacts();
		for (int j = 0; j < numContacts; ++j) {
			btManifoldPoint pt = contactManifold->getContactPoint(j);
			if (pt.getDistance() < 0.0f) {
				artemis::Entity& aent = em->getEntity(ps_Entities.at(a));
				artemis::Entity& bent = em->getEntity(ps_Entities.at(b));
				
				NodeComponent* ANode = nodeMapper.get(aent);
				NodeComponent* BNode = nodeMapper.get(bent);

				if ((ANode->flags & COMPONENT_COLIDER) && (BNode->flags & COMPONENT_COLIDER)) {

					CollisionComponent* AComp = (CollisionComponent*)aent.getComponent<CollisionComponent>();
					CollisionComponent* BComp = (CollisionComponent*)bent.getComponent<CollisionComponent>();
					CollisionData apn = CollisionData(pt.getPositionWorldOnA(), -pt.m_normalWorldOnB);
					CollisionData bpn = CollisionData(pt.getPositionWorldOnB(), pt.m_normalWorldOnB);
					AComp->collisions[bent.getId()] = apn;
					BComp->collisions[aent.getId()] = bpn;
					

					/*if (ANode->flags & COMPONENT_SPRING)
						springUp(aent, bent);
					if (ANode->flags & COMPONENT_BUTTON) {
						ButtonComponent* bc = (ButtonComponent*)aent.getComponent<ButtonComponent>();
						bc->collided = true;
					}
					if (BNode->flags & COMPONENT_BUTTON) {
						ButtonComponent* bc = (ButtonComponent*)bent.getComponent<ButtonComponent>();
						bc->collided = true;
					}*/
				}
				//std::cout << "SHAPE A IS: " << shapeA << " SHAPE B IS: " << shapeB << std::endl; 
			}
		}
	}
	dynamicsWorld->stepSimulation(1.f / 60.f, 10);

}

void PhysicsSystem::onCollision(btCollisionObject * a, btCollisionObject * b, btManifoldPoint p)
{

	
}

void PhysicsSystem::togglePlayMode()
{
	btCollisionObjectArray& objArray = dynamicsWorld->getCollisionObjectArray();
	for (int i = 0; i < objArray.size(); ++i) {	
		
		btCollisionObject* obj = objArray[i];
		TransformComponent* tc = (TransformComponent*)em->getEntity(ps_Entities.at(obj)).getComponent<TransformComponent>();
		obj->setWorldTransform(glmToBulletTransform(tc->world));

	}
	//for (size_t i = 0; i < ps_Entities.size(); ++i) {
	//	TransformComponent* tc = (TransformComponent*)ps_Entities[i].entity->getComponent<TransformComponent>();
	//	ps_Entities[i].colObj->setWorldTransform(glmToBulletTransform(tc->world));
	//	NodeComponent* nc = (NodeComponent*)ps_Entities[i].entity->getComponent<NodeComponent>();

		//ts->SQTTransform(nc, fake);
		//ts->SQTTransform((NodeComponent*)ps_Entities[i].entity->getComponent<NodeComponent>(), fake);
	//}
}

btTransform PhysicsSystem::glmToBulletTransform(glm::mat4 t)
{
	btMatrix3x3 rotation = 
		//btMatrix3x3(
		//t[0][0], t[0][1], t[0][2],
		//t[1][0], t[1][1], t[1][2],
		//t[2][0], t[2][1], t[2][2]);

	btMatrix3x3(
		t[0][0], t[1][0], t[2][0],
		t[0][1], t[1][1], t[2][1],
		t[0][2], t[1][2], t[2][2]);

	btVector3 position = btVector3(t[3][0], t[3][1], t[3][2]);
	return btTransform(rotation, position);
}
