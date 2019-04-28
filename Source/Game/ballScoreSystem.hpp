/*
BallScoringSystem Copyright (C) by Mike Murrell 2019
ballscoring system = singleton component that has
- player1 score player 2 score
- takes in the balls
- - processEntity = process the balls and check for hittype::goal
- - if goal, the coll should tell u what node it was so just do teh score for that
- - - ball spawn component should delete the ball
*/
#pragma once
#include "ballComponents.h"
#include "../Utility/nodeComponent.hpp"

class BallScoreSystem : public artemis::EntityProcessingSystem {
private: 
	artemis::ComponentMapper<BallComponent>  ballMapper;
	BallScoreComponent* scoreComp;
	BallSpawnComponent* spawner;

public:
	BallScoreSystem() {
		addComponentType<BallComponent>();
	}
	void initialize() {
		ballMapper.init(*world);
		spawner = (BallSpawnComponent*)world->getSingleton()->getComponent<BallSpawnComponent>();
		scoreComp = (BallScoreComponent*)world->getSingleton()->getComponent<BallScoreComponent>();
	}
	void processEntity(artemis::Entity &e) {
		BallComponent* ball = ballMapper.get(e);
		if (ball->hitType == HitType::Goal) {
			scoreComp->scores[ball->goalHit]++;
			spawner->state = BallSpawnState::Remove;
			spawner->removedBallID = e.getId();
		}
	}
};