#pragma once
/* Game System Copyright (C) by Mike Murrell 
iirc this is rush-code from GGJ idk if i will
use or not
*/
#include "scene.h" 
#include "gameComponent.hpp"

#include "ballCollisionSystem.hpp"
#include "ballMovementSystem.hpp"
#include "ballSpawnSystem.hpp"
#include "ballScoreSystem.hpp"
#include "../Physics/collisionSystem.hpp"

class GameSystem : public artemis::EntityProcessingSystem {
private:
	artemis::ComponentMapper<GameComponent> gameMapper;
	artemis::ComponentMapper<GlobalController> controlMapper;

	GameComponent* game;
	GlobalController* controller;

	artemis::EntityManager* em;
	artemis::SystemManager* sm;

	RenderSystem*	rs;
	TransformSystem* ts;
	PhysicsSystem* ps;
	EngineUISystem* ui;
	AnimationSystem* as;

	CharacterController* cc;
	ControllerSystem* input;

	CollisionSystem* cs;
	BallSpawnSystem* spawner;
	BallScoreSystem* scorer;
	BallMovementSystem* bms;
	BallCollisionSystem* bcs;

public: 
	GameSystem();
	~GameSystem();

	void initialize();
	void processEntity(artemis::Entity &e);
	void updateInput();
	void togglePlayMode();

};
