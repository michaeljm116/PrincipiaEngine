#pragma once
/* Physics System Copyright (C) by Mike Murrell 
*/

#include "../pch.h"
#include "../Utility/componentIncludes.h"


#include "../Rendering/renderSystem.h"
#include "../Game/transformSystem.h"
//#include "../Game/scene.h"

struct PhysicsEntity {
	artemis::Entity* entity;
	btCollisionObject* colObj;
	
	bool compare(btCollisionObject* c) {
		return c == colObj;
	}
	//if  collision with entity tell both entities who you collided with and where at
};

class PhysicsSystem : public artemis::EntityProcessingSystem
{
private:
	artemis::ComponentMapper<NodeComponent> nodeMapper;
	artemis::ComponentMapper<RigidBodyComponent> rbMapper;
	artemis::ComponentMapper<PrimitiveComponent> objMapper;


	btAlignedObjectArray<btCollisionShape*> collisionShapes;

	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btBroadphaseInterface* overlappingPairCache;
	btSequentialImpulseConstraintSolver* solver;
	btSimpleDynamicsWorld* dynamicsWorld;

	artemis::EntityManager* em;
	artemis::SystemManager* sm;
	RenderSystem* rs;
	TransformSystem* ts;

	//std::vector<PhysicsEntity> ps_Entities;

	std::unordered_map<const btCollisionObject*, int> ps_Entities;

	sqt fake;
public:
	PhysicsSystem();
	~PhysicsSystem();
	//void initialize();

	void initialize();
	void processEntity(artemis::Entity& e);
	void addGroundForNow();
	void addNode(NodeComponent* node);
	void deleteNode(NodeComponent* node);
	void addCol(NodeComponent* node);
	void update();
	void onCollision(btCollisionObject* a, btCollisionObject* b, btManifoldPoint p);
	btRigidBody* getRigidBody(artemis::Entity& e) {
		RigidBodyComponent* rbc = rbMapper.get(e);
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[rbc->coaIndex];
		return btRigidBody::upcast(obj);
	}
	btCollisionObject* getCollisionObject(artemis::Entity& e) {
		RigidBodyComponent* rbc = rbMapper.get(e);
		return dynamicsWorld->getCollisionObjectArray()[rbc->coaIndex];
	}

	void applyForce(btRigidBody* b, btVector3 f) {
 		b->applyCentralForce(f);
	}
	void applyImpulse(btRigidBody* b, btVector3 i) {
		b->applyCentralImpulse(i);
	}
	void applyImpulse(artemis::Entity& ent) {
		rbMapper.get(ent)->impulse = true;
	}
	void applyMovement(btCollisionObject* b, btVector3 f) {
		//btVector3 m = b->getWorldTransform().getOrigin() + f;
		b->getWorldTransform().setOrigin(b->getWorldTransform().getOrigin() + f);
	}

	void springUp(artemis::Entity& a, artemis::Entity& b) {
		RigidBodyComponent* arb = rbMapper.get(a);
		RigidBodyComponent* brb = rbMapper.get(b);
		btVector3 av = arb->trans.getOrigin();
		btVector3 bv = brb->trans.getOrigin();

		if (av.getY() > (bv.getY() + 0.1f))
			arb->impulse = true;
		else if (bv.getY() > (av.getY() + 0.1f))
			brb->impulse = true;
	}
	bool CheckGrounding(btCollisionObject* b) {
		btVector3 pos = b->getWorldTransform().getOrigin();
		btVector3 to(pos.getX(), -10000.f, pos.getZ());
		btCollisionWorld::ClosestRayResultCallback res(pos, to);
		dynamicsWorld->rayTest(pos, to, res);
		if (res.hasHit()) {
			if (pos.distance2(res.m_hitPointWorld) < 1.2f) {
				//res.m_collisionObject.
				return true;
			}
			//res.m_collisionObject
		}
		return false;
	}


	void togglePlayMode();
	btTransform glmToBulletTransform(glm::mat4);

};
