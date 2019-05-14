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

#include "ballComponents.hpp"
#include "../Physics/physicsSystem.h"

class BallMovementSystem : public artemis::EntityProcessingSystem {
private:
	artemis::ComponentMapper<BallComponent>		 ballMapper;
	artemis::ComponentMapper<RigidBodyComponent> rbMapper;
public:
	BallMovementSystem();
	void initialize();
	void processEntity(artemis::Entity &e);
	btVector3 glm2bulVec3(const glm::vec3& in);
};