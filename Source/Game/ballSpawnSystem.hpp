/*
/* BallSpawnSystem Copyright (C) by Mike Murrell 2019
This system controls how the balls spawn
- First it checks to see if it has any balls to spawn
-	if true then checks the last time a ball has spawned
-		if enough time then it randomly spawns a ball + resettimer
-	if not enough time then checks if any balls in field
-		if no balls on field then spawn a ball + resettimer
-	iterate timer;
- iterate nothing
*/

#pragma once
#include <Artemis/Artemis.h>
#include "scene.h"
#include "ballComponents.h"
#include "ballCollisionSystem.hpp"
#include "ballMovementSystem.hpp"


class BallSpawnSystem : public artemis::EntityProcessingSystem {
private:
	artemis::ComponentMapper<BallSpawnComponent> bsMapper;
	BallSpawnComponent* bs;
public:
	BallSpawnSystem() {
		addComponentType<BallSpawnComponent>();
	}
	void initialize() {
		bsMapper.init(*world);
		bs = bsMapper.get(*world->getSingleton());
	}
	void processEntity(artemis::Entity &e) {
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
	void spawnBall() {
		
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
		BallCollisionSystem* bcs = (BallCollisionSystem*)world->getSystemManager()->getSystem<BallCollisionSystem>();
		BallMovementSystem* bms = (BallMovementSystem*)world->getSystemManager()->getSystem<BallMovementSystem>();

		bcs->change(*e);
		bms->change(*e);
	}

	void removeBall() {
		bs->activeBalls--;
		bs->timer = 0.f;
		NodeComponent* nc = (NodeComponent*)world->getEntity(bs->removedBallID).getComponent<NodeComponent>();
		SCENE.deleteNode(nc);
	}
};