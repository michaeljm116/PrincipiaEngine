#pragma once
/* Game System Copyright (C) by Mike Murrell 
iirc this is rush-code from GGJ idk if i will
use or not
*/
#include "../scene.h" 
#include "applicationComponents.h"
#include "../Gameplay/ballCollisionSystem.h"
#include "../Gameplay/ballMovementSystem.h"
#include "../Gameplay/ballSpawnSystem.h"
#include "../Gameplay/ballScoreSystem.h"
#include "../Gameplay/characterController.h"
#include "../Physics/collisionSystem.h"
#include "../Audio/audioSystem.h"

class GameSystem : public artemis::EntityProcessingSystem {
private:
	artemis::ComponentMapper<GameComponent> gameMapper;
	artemis::ComponentMapper<ControllerComponent> controllerMapper;

	artemis::EntityManager* em;
	artemis::SystemManager* sm;

	RenderSystem*	rs;
	TransformSystem* ts;
	PhysicsSystem* ps;
	EngineUISystem* ui;
	AnimationSystem* as;

	CharacterController* cc;

	CollisionSystem* cs;
	BallSpawnSystem* spawner;
	BallScoreSystem* scorer;
	BallMovementSystem* bms;
	BallCollisionSystem* bcs;
	AudioSystem* audio;


	std::vector<GUINumberComponent*> goals;

public: 
	GameSystem();
	~GameSystem();

	void initialize();
	void added(artemis::Entity &e);
	void removed(artemis::Entity &e); 
	void processEntity(artemis::Entity &e);
	void findGoals();
};
