#pragma once
/*
BallCollisionSystem Copyright (C) by Mike Murrell 2019
Updates the ball's data depending on what it collides with
- Takes in BallComponent
- Takes in CollisionComponent
- process entity:
- - check for collisionflag
- - - if player, adjust ball speed based on player hit power
- - - if goal, set type to goal
- - - if wall, just set type to  wall
*/

#include "ballComponents.hpp"
#include "characterComponent.hpp"
#include "../Physics/physicsSystem.h"

class BallCollisionSystem : public artemis::EntityProcessingSystem {
private:
	artemis::ComponentMapper<BallComponent> ballMapper;
	artemis::ComponentMapper<CollisionComponent> colMapper;
public:
	BallCollisionSystem();
	void initialize();
	void processEntity(artemis::Entity &e);

	glm::vec3 bul2glmVec3(const btVector3& in);
	glm::vec3 ballReflection(glm::vec3 balldir, btVector3 colpos, TransformComponent* tc);
};