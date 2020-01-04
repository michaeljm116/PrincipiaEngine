#pragma once
/* Game System Copyright (C) by Mike Murrell 
iirc this is rush-code from GGJ idk if i will
use or not
*/
#include "../scene.h" 
#include "applicationComponents.h"
#include "../Gameplay/characterController.h"
#include "../Gameplay/enemyController.h"
#include "../Audio/audioSystem.h"
#include "../Gameplay/projectileMovementSystem.h"
#include "../Gameplay/characterRotationSystem.h"
#include "../Physics/immovableSystem.h"


using namespace Principia;
class GameSystem : public artemis::EntityProcessingSystem {
private:
	artemis::ComponentMapper<GameComponent> gameMapper;
	artemis::ComponentMapper<ControllerComponent> controllerMapper;

	artemis::EntityManager* em;
	artemis::SystemManager* sm;

	RenderSystem*	rs;
	TransformSystem* ts;
	EngineUISystem* ui;
	AnimationSystem* as;

	ImmovableSystem* sysImmovable;
	CharacterController* cc;
	CharacterRotationSystem* crs;
	EnemyController* ec;
	ProjectileMovementSystem* pms;

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
