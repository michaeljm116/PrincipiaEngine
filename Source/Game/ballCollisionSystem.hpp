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

#include "ballComponents.h"
#include "characterComponent.hpp"
#include "../Physics/physicsSystem.h"

class BallCollisionSystem : public artemis::EntityProcessingSystem {
private:
	artemis::ComponentMapper<BallComponent> ballMapper;
	artemis::ComponentMapper<CollisionComponent> colMapper;
public:
	BallCollisionSystem() {
		addComponentType<BallComponent>();
		addComponentType<CollisionComponent>();
	}
	void initialize() {
		ballMapper.init(*world);
		colMapper.init(*world);
	}
	void processEntity(artemis::Entity &e) {
		BallComponent* ball = ballMapper.get(e);
		CollisionComponent* colMap = colMapper.get(e);
		if (ball->timeSinceHit > 0.1f && colMap->collisions.size() > 0) {
			for (auto col : colMap->collisions) {
				if (col.second.state == CollisionState::Continue || col.second.state == CollisionState::Start) {
					col.second.state = CollisionState::Exit;
					artemis::Entity& colider = world->getEntity(col.first);
					NodeComponent* nc = (NodeComponent*)colider.getComponent<NodeComponent>();
					if (nc->tags & TAG_NONE)
						break;
					else if (nc->tags & TAG_PLAYER) {
						ball->hitType = HitType::Player;
						CharacterComponent* cc = (CharacterComponent*)colider.getComponent<CharacterComponent>();
						ball->speed = cc->hitPower;
						ball->direction = glm::reflect(ball->direction, bul2glmVec3(col.second.norm));
						ball->timeSinceHit = 0.f;
						//REFLECT
					}
					else if (nc->tags & TAG_WALL) {
						ball->hitType = HitType::Wall;
						ball->direction = glm::reflect(ball->direction, bul2glmVec3(col.second.norm));
						ball->timeSinceHit = 0.f;
						//REFLECT
					}
					else if (nc->tags & TAG_GOAL) {
						ball->hitType = HitType::Goal;
						ball->goalHit = (nc->name.at(nc->name.length() - 1) - '1');
						//SCORE
					}
				}
			}
		}
		//you need an oncollisionexit thing
		ball->timeSinceHit += world->getDelta();
	}

	glm::vec3 bul2glmVec3(const btVector3& in) {
		return glm::vec3(in[0], in[1], in[2]);
	}
};