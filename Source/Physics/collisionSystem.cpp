#include "collisionSystem.h"
#include "collisionTests.h"
#include "../Utility/helpers.h"

namespace Principia {
	CollisionSystem::CollisionSystem()
	{
		addComponentType<CollisionComponent>();
		addComponentType<TransformComponent>();
		//addComponentType<DynamicComponent>();
	}

	CollisionSystem::~CollisionSystem()
	{
	}

	void CollisionSystem::initialize()
	{
		colMapper.init(*world);
		transMapper.init(*world);
		//grid = (GridComponent*)world->getSingleton()->getComponent<GridComponent>();
		bulletInit();
	}

	void CollisionSystem::begin()
	{
		bulletUpdate();
	}


	void CollisionSystem::processEntity(artemis::Entity & e)
	{
		bulletProcessEntity(e);

		/*
		//Goes through every object and checks for collision
		TransformComponent* tc = transMapper.get(e);
		CollisionComponent* col = colMapper.get(e);
		col->position = tc->world[3];
		//col->extents = rotateBounds(tc->global.rotation, col->extents);

		//This is for all static objects ///////////////do later right now u finna test spheres
		checkStaticCollision(e);		
		*/

	}

	void CollisionSystem::added(artemis::Entity& e)
	{
		bulletAdded(e);
	}

	void CollisionSystem::removed(artemis::Entity& e)
	{
		bulletRemoved(e);
	}

	void CollisionSystem::checkDynamicCollisions()
	{
		auto* actives = getActives();
		int c = actives->getCount();

		for (int i = 0; i < c - 1; ++i) {
			auto a = actives->get(i);
			for (int j = i + 1; j < c; ++j) {
				auto b = actives->get(j);
				checkCollision(*a, *b);
			}
		}
	}

	void CollisionSystem::checkStaticCollision(artemis::Entity & e)
	{
		std::vector<artemis::Entity*> collisions;
		CollisionComponent* cc = colMapper.get(e);
		GridBlock gb = GridBlock(cc->position, cc->extents);
		if (!gb.verify(grid->size))
			return;
		for (int r = gb.leftx; r < gb.rightx; r += gb.itr) {
			for (int c = gb.downy; c < gb.upy; c += gb.itr) {
				if (grid->grid[r][c] != nullptr) {
					if (std::find(collisions.begin(), collisions.end(), grid->grid[r][c]) == collisions.end())
						collisions.push_back(grid->grid[r][c]);
				}
			}
		}

		//check collision list for collisions
		if (collisions.size() > 0) {
			for (auto& c : collisions)
				checkCollision(e, *c);
		}
	}

	void CollisionSystem::checkCollision(artemis::Entity & a, artemis::Entity & b)// const
	{
		CollisionComponent* ccA = colMapper.get(a);
		CollisionComponent* ccB = colMapper.get(b);
		CollisionData cda = CollisionData(b.getId());
		CollisionData cdb = CollisionData(a.getId());
		
		if (CollisionTests::ColTests[CollisionTests::Convert(ccA->type, ccB->type)](ccA, ccB, cda, cdb)) {
			CollidedComponent* cwa = (CollidedComponent*)a.getComponent<CollidedComponent>();
			CollidedComponent* cwb = (CollidedComponent*)b.getComponent<CollidedComponent>();

			
			cwa == nullptr ?
				a.addComponent(new CollidedComponent(cda))
				:
				cwa->update(cda);

			cwb == nullptr ?
				b.addComponent(new CollidedComponent(cdb))
				:
				cwb->update(cdb);

			a.refresh();
			b.refresh();
		}
	}

	glm::vec3 CollisionSystem::rotateBounds(const glm::quat & m, const glm::vec3 & extents)
	{
		//set up cube
		glm::vec3 v[8];
		v[0] = extents;
		v[1] = glm::vec3(extents.x, extents.y, -extents.z);
		v[2] = glm::vec3(extents.x, -extents.y, -extents.z);
		v[3] = glm::vec3(extents.x, -extents.y, extents.z);
		v[4] = glm::vec3(-extents);
		v[5] = glm::vec3(-extents.x, -extents.y, extents.z);
		v[6] = glm::vec3(-extents.x, extents.y, -extents.z);
		v[7] = glm::vec3(-extents.x, extents.y, extents.z);

		//transform them
#pragma omp parallel for
		for (int i = 0; i < 8; ++i) {
			v[i] = abs(m * v[i]);// glm::vec4(v[i], 1.f));

		}

		//compare them
		glm::vec3 vmax = glm::vec3(FLT_MIN);
		for (int i = 0; i < 8; ++i) {
			vmax.x = tulip::max(vmax.x, v[i].x);
			vmax.y = tulip::max(vmax.y, v[i].y);
			vmax.z = tulip::max(vmax.z, v[i].z);
		}

		return vmax;
	}

	void CollisionSystem::bulletInit()
	{
		collisionConfiguration = new btDefaultCollisionConfiguration();
		dispatcher = new btCollisionDispatcher(collisionConfiguration);
		overlappingPairCache = new btDbvtBroadphase();
		solver = new btSequentialImpulseConstraintSolver;
		dynamicsWorld = new btSimpleDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
		dynamicsWorld->setGravity(btVector3(0, -10, 0));

		//Add ground for now
		btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);// new btBoxShape(btVector3(btScalar(50.), btScalar(1.), btScalar(50.)));
		collisionShapes.push_back(groundShape);
		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(8, -6 + 1.5f, 8));
		btScalar mass(0.);
		btVector3 localInertia(0, 0, 0);
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		dynamicsWorld->addRigidBody(body);
	}

	void CollisionSystem::bulletAdded(artemis::Entity& e)
	{
		auto* col = colMapper.get(e);
		auto* tc = transMapper.get(e);
		btVector3 inertia = btVector3(0.f, 0.f, 0.f);
		btScalar mass =  col->mass;
		btCollisionShape* shape;

		switch (col->type) {
		case CollisionType::Box: shape = new btBoxShape(g2bv3(col->extents)); break;
		case CollisionType::Sphere: shape = new btSphereShape(btScalar(col->extents.x)); break;
		case CollisionType::Capsule: shape = new btCapsuleShape(col->extents.x, col->extents.y); break;
		case CollisionType::Other: break;
		default: break;
		}

		collisionShapes.push_back(shape);
		btDefaultMotionState* ms = new btDefaultMotionState(g2bt(tc->TRM));
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, ms, shape, inertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		dynamicsWorld->addRigidBody(body);
		col->body = body;
	}

	void CollisionSystem::bulletRemoved(artemis::Entity& e)
	{
		auto* col = colMapper.get(e);
		dynamicsWorld->removeRigidBody(col->body);

	}

	void CollisionSystem::bulletUpdate()
	{
		//int numManifolds = dispatcher->getNumManifolds();
		//for (int i = 0; i < numManifolds; ++i) {
		//	btPersistentManifold* contactManifold = dispatcher->getManifoldByIndexInternal(i);
		//	const btCollisionObject* a = contactManifold->getBody0();
		//	const btCollisionObject* b = contactManifold->getBody1();
		//	int numContacts = contactManifold->getNumContacts();
		//	for (int j = 0; j < numContacts; ++j) {
		//		btManifoldPoint pt = contactManifold->getContactPoint(j);
		//		if (pt.getDistance() < 0.0f) {
		//			//insert collisiondata stuff

		//		}
		//	}
		//}


		dynamicsWorld->stepSimulation(0.016666666666666666f, 10.f);
	}

	void CollisionSystem::bulletProcessEntity(artemis::Entity& e)
	{
		auto* col = colMapper.get(e);
		auto* tc = transMapper.get(e);
		auto& trans = col->body->getWorldTransform();
		tc->local.rotation = b2gq(trans.getRotation());
		tc->local.position = b2gv4(trans.getOrigin());

	}

	void CollisionSystem::bulletOnCollision(btCollisionObject* a, btCollisionObject* b, btManifoldPoint p)
	{
	}
}