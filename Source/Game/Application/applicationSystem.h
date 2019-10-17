#pragma once
#include "applicationComponents.h"
#include "controllerSystem.h"
#include "gameSystem.h"
#include "titleSystem.h"
#include "menuSystem.h"
#include "pauseSystem.h"
#include "../Utility/bvhSystem.h"

class ControllerSystem;

class ApplicationSystem : public artemis::EntityProcessingSystem {
private:
	artemis::ComponentMapper<ApplicationComponent> appMapper;
	artemis::ComponentMapper<GlobalController> controlMapper;

//	ControllerSystem* controllers;

	GameSystem* game;
	TitleSystem* title;
	MenuSystem* menu;
	PauseSystem* pause;

	RenderSystem* rs;
	AnimationSystem* as;
	EngineUISystem* ui;
	ControllerSystem* controllers;
	BvhSystem* bvh;

public:
	ApplicationSystem();
	~ApplicationSystem();
	void initialize();
	void processEntity(artemis::Entity &e);

	void instantGameStart();
	void toggleEditor(AppState& s);
	void toggleAppState(AppState& s);
	void togglePause(AppState& s);
};
