#include "../pch.h"
#include "collisionSystem.h"
#include "collisionTests.h"
#include "../Utility/helpers.h"
#include "../Utility/nodeComponent.hpp"
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <set>
#include <thread>


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
	void randomfunc() {
		std::cout << "MUlti thread me bby ooo yeah";
	}
	void CollisionSystem::begin()
	{
		//physicsThread = new std::thread(bulletUpdate);
		//physicsThread = std::thread(&bulletUpdate);
		//std::thread bob(bulletUpdate);
		//bob.join();
		bulletUpdate();
	}

	void CollisionSystem::end()
	{
		//physicsThread.join();
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
		if(!world->getShutdown())
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
		//GridBlock gb = GridBlock(cc->position, cc->extents);
		//if (!gb.verify(grid->size))
		//	return;
		//for (int r = gb.leftx; r < gb.rightx; r += gb.itr) {
		//	for (int c = gb.downy; c < gb.upy; c += gb.itr) {
		//		if (grid->grid[r][c] != nullptr) {
		//			if (std::find(collisions.begin(), collisions.end(), grid->grid[r][c]) == collisions.end())
		//				collisions.push_back(grid->grid[r][c]);
		//		}
		//	}
		//}

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
		physics = new Cmp_Physics();
		physics->collisionConfiguration = new btDefaultCollisionConfiguration();
		physics->dispatcher = new btCollisionDispatcher(physics->collisionConfiguration);
		physics->overlappingPairCache = new btDbvtBroadphase();
		physics->solver = new btSequentialImpulseConstraintSolver;
		//physics->dynamicsWorld = new btSimpleDynamicsWorld(physics->dispatcher, physics->overlappingPairCache, physics->solver, physics->collisionConfiguration);
		physics->dynamicsWorld = new btDiscreteDynamicsWorld(physics->dispatcher, physics->overlappingPairCache, physics->solver, physics->collisionConfiguration);
		physics->dynamicsWorld->setGravity(btVector3(0, -100, 0));
		world->getSingleton()->addComponent(physics);


		//Add ground for now
		//artemis::Entity* ground_entity = &world->getEntityManager()->create();
		//ground_entity->addComponent(new CollisionComponent());
		//auto* ground_component = (CollisionComponent*)ground_entity->getComponent<CollisionComponent>();
		//ground_entity->addComponent(new TransformComponent(glm::vec3(0), glm::vec3(24.0, 0, 24.0)));

		//btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);// new btBoxShape(btVector3(btScalar(50.), btScalar(1.), btScalar(50.)));
		//btTransform groundTransform;
		//groundTransform.setIdentity();
		//groundTransform.setOrigin(btVector3(24, 0, 24));
		//btScalar mass(0.);
		//btVector3 localInertia(0, 0, 0);
		//btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		//btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		//btRigidBody* body = new btRigidBody(rbInfo);

		//ground_component->body = body;
		//physics->collisionShapes.push_back(groundShape);
		//physics->dynamicsWorld->addRigidBody(body);
	}

	void CollisionSystem::bulletAdded(artemis::Entity& e)
	{
		auto* col = colMapper.get(e);
		auto* tc = transMapper.get(e);
		btVector3 inertia = btVector3(0.f, 0.f, 0.f);
		btScalar mass =  col->mass;
		btCollisionShape* shape;

		switch (col->type) {
		case CollisionType::Box:	 shape = new btBoxShape(g2bv3(col->extents)); break;
		case CollisionType::Sphere:  shape = new btSphereShape(btScalar(col->extents.x)); break;
		case CollisionType::Capsule: shape = new btCapsuleShape(col->extents.x, col->extents.y); break;
		case CollisionType::Ghost:	 shape = new btSphereShape(btScalar(col->extents.x)); break;
		case CollisionType::Plane:	 
			shape = new btStaticPlaneShape(btVector3(0, 1, 0), 0); mass = 0.f; break;
		default: break;
		}

		physics->collisionShapes.push_back(shape);
		//if (col->type == CollisionType::Plane)
		//bullet_trans.setOrigin(btVector3(tc->local.position.x, tc->local.position.y + tc->local.scale.y + 0.5f, tc->local.position.z));
		btDefaultMotionState* ms = new btDefaultMotionState(g2bt(tc->TRM));
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, ms, shape, inertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		if(mass == 10.f)
			body->setGravity(btVector3(0, 0, 0));
		if(col->type == CollisionType::Ghost)body->setCollisionFlags(btCollisionObject::CO_GHOST_OBJECT);

		auto isDynamic = ((DynamicComponent*)e.getComponent<DynamicComponent>() != nullptr);
		if(isDynamic)body->setCollisionFlags(body->getCollisionFlags() & ~1);// CF_STATIC_OBJECT);

		physics->dynamicsWorld->addRigidBody(body);
		col->body = body;
		entityMap.insert(std::make_pair(body, e.getId()));

		auto* nc = (NodeComponent*)e.getComponent<NodeComponent>();
		auto n = nc->name;
		auto num = e.getId();
	}

	void CollisionSystem::bulletRemoved(artemis::Entity& e)
	{
		auto* col = colMapper.get(e);
		physics->dynamicsWorld->removeRigidBody(col->body);
		entityMap.erase(col->body);
	}

	void CollisionSystem::bulletUpdate()
	{

		physics->dynamicsWorld->stepSimulation(time_step, 0);
		//physics->dynamicsWorld->stepSimulation(time_step, 1);

		int numManifolds = physics->dispatcher->getNumManifolds();
		for (int i = 0; i < numManifolds; ++i) {
			btPersistentManifold* contactManifold = physics->dispatcher->getManifoldByIndexInternal(i);
			const btCollisionObject* a = contactManifold->getBody0();
			const btCollisionObject* b = contactManifold->getBody1();
			int numContacts = contactManifold->getNumContacts();
			for (int j = 0; j < numContacts; ++j) {
				btManifoldPoint pt = contactManifold->getContactPoint(j);
				if (pt.getDistance() < 0.0f) {
					int a_id = entityMap.at(a);
					int b_id = entityMap.at(b);

					artemis::Entity& a_ent = world->getEntity(a_id);
					artemis::Entity& b_ent = world->getEntity(b_id);

					CollisionData a_cd = CollisionData(a_id, b2gv3(pt.m_positionWorldOnA), b2gv3(pt.m_normalWorldOnB));
					CollisionData b_cd = CollisionData(b_id, b2gv3(pt.m_positionWorldOnB), b2gv3(pt.m_normalWorldOnB));

					CollidedComponent* a_cw = (CollidedComponent*)a_ent.getComponent<CollidedComponent>();
					CollidedComponent* b_cw = (CollidedComponent*)b_ent.getComponent<CollidedComponent>();


					a_cw == nullptr ?
						a_ent.addComponent(new CollidedComponent(b_cd))
						:
						a_cw->update(b_cd);

					b_cw == nullptr ?
						b_ent.addComponent(new CollidedComponent(a_cd))
						:
						b_cw->update(a_cd);

					a_ent.refresh();
					b_ent.refresh();

				}
			}
		}
		

		//physics->dynamicsWorld->stepSimulation(world->getDelta());// , 10.f);
	}

	void CollisionSystem::bulletProcessEntity(artemis::Entity& e)
	{
		auto* tc = transMapper.get(e);
		auto& trans = colMapper.get(e)->body->getWorldTransform();
		tc->local.rotation = b2gq(trans.getRotation());
		tc->local.position = b2gv4(trans.getOrigin()); 
	}

	void CollisionSystem::bulletOnCollision(btCollisionObject* a, btCollisionObject* b, btManifoldPoint p)
	{
	}
}