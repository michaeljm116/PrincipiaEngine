#pragma once
/*
BallMovementSystem Copyright (C) by Mike Murrell 2019
the ball movement system controlls the behavor of its movement based on the ball data
 - check if the ball has been hit 
 - - if not then update position based on speed
 - - if so then 
 - - - if type = wall, reflect direction
 - - - if type = player, reflect direction + adjust speed based on players hit power
 - - - if type = goal, nothing really... the scoring system will handle this

*/

#include "ballComponents.h"
#include "../Physics/physicsSystem.h"

class BallMovementSystem : public artemis::EntityProcessingSystem {
private: 
	artemis::ComponentMapper<BallComponent>		 ballMapper;
	artemis::ComponentMapper<RigidBodyComponent> rbMapper;
public:
	BallMovementSystem() {
		addComponentType<BallComponent>();
		addComponentType<RigidBodyComponent>();
	}
	void initialize() {
		ballMapper.init(*world);
		rbMapper.init(*world);
	}
	void processEntity(artemis::Entity &e) {
		BallComponent*		ball	= ballMapper.get(e);
		RigidBodyComponent* rbc		= rbMapper.get(e);
		
		btTransform& trans = rbc->body->getWorldTransform();
		btVector3 pos = trans.getOrigin();
		float delta = world->getDelta();
		if (delta > 1.f)
			int a = 4;
		else
		pos += glm2bulVec3(ball->direction * ball->speed * world->getDelta());
		trans.setOrigin(pos);
	}

	btVector3 glm2bulVec3(const glm::vec3& in) {
		return btVector3(in[0], in[1], in[2]);
	}
};