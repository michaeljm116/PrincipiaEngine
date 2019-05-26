#include "ballSpawnSystem.h"
#include "../pch.h"

BallSpawnSystem::BallSpawnSystem() {
	addComponentType<BallSpawnComponent>();
}

void BallSpawnSystem::initialize() {
	bsMapper.init(*world);
	bs = bsMapper.get(*world->getSingleton());
}

void BallSpawnSystem::processEntity(artemis::Entity &e) {
	switch (bs->state)
	{
	case BallSpawnState::Ready:
		spawnBall();
		bs->state = BallSpawnState::Waiting;
		break;
	case BallSpawnState::Waiting:
		if (bs->activeBalls < bs->maxballs) {
			if (bs->timer > bs->maxTime)
				bs->state = BallSpawnState::Ready;
		}
		bs->timer += world->getDelta();
		break;
	case BallSpawnState::Remove:
		removeBall();
		bs->state = BallSpawnState::Ready;
		break;
	default:
		break;
	}
}

//Randomly spawns a sphere at a distnace and etc...
//needs render/physics/etc... without talking to them directly
void BallSpawnSystem::spawnBall() {

	artemis::Entity* e = SCENE.createGameShape("Ball", glm::vec3(0.f, 1.f, 0.f), glm::vec3(1.f), 0, -1, true);
	float x;
	float y;
	e->addComponent(new BallComponent());
	e->addComponent(new CollisionComponent());
	NodeComponent* nc = (NodeComponent*)e->getComponent<NodeComponent>();
	nc->flags |= COMPONENT_COLIDER;
	SCENE.insertRigidBody(nc);
	//SCENE.rs->change(*e);
	bs->activeBalls++;
	bs->timer = 0.f;

	e->refresh();
	//BallCollisionSystem* bcs = (BallCollisionSystem*)world->getSystemManager()->getSystem<BallCollisionSystem>();
	//BallMovementSystem* bms = (BallMovementSystem*)world->getSystemManager()->getSystem<BallMovementSystem>();

	//bcs->change(*e);
	//bms->change(*e);
}

void BallSpawnSystem::removeBall() {
	bs->activeBalls--;
	bs->timer = 0.f;
	NodeComponent* nc = (NodeComponent*)world->getEntity(bs->removedBallID).getComponent<NodeComponent>();
	SCENE.deleteNode(nc);
}