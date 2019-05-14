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
#include "ballComponents.hpp"
#include "../Utility/nodeComponent.hpp"
#include "../Rendering/renderComponents.hpp"
#include "../Rendering/renderSystem.h"

class BallScoreSystem : public artemis::EntityProcessingSystem {
private:
	artemis::ComponentMapper<BallScoreComponent> scorer;
	artemis::ComponentMapper<GUINumberComponent> guiMapper;

public:
	BallScoreSystem();
	void initialize();
	void processEntity(artemis::Entity &e);
};