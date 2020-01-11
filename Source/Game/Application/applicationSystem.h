#pragma once
#include "applicationComponents.h"
#include "controllerSystem.h"
#include "gameSystem.h"
#include "titleSystem.h"
#include "menuSystem.h"
#include "pauseSystem.h"
#include "../Utility/bvhSystem.h"
#include "../Physics/gridSystem.h"
#include "../Physics/collisionSystem.h"
#include "../Physics/collidedWithSystem.h"
#include "../Physics/immovableSystem.h"
#include "../Gameplay/gameSceneSystem.h"

//class ControllerSystem;

using namespace Principia;
class ApplicationSystem : public artemis::EntityProcessingSystem {
private:
	artemis::ComponentMapper<ApplicationComponent> appMapper;
	artemis::ComponentMapper<GlobalController> controlMapper;

//	ControllerSystem* controllers;

	GameSystem* game;
	TitleSystem* title;
	MenuSystem* menu;
	PauseSystem* pause;
	AnimationSystem* as;
	ControllerSystem* controllers;
	BvhSystem* bvh;
	GridSystem* sysGrid;
	CollisionSystem* col;
	CollidedWithSystem* cws;
	GameSceneSystem* gss;

public:

	RenderSystem* rs;
	EngineUISystem* ui;


	ApplicationSystem();
	~ApplicationSystem();
	void initialize();
	void processEntity(artemis::Entity &e);

	void instantGameStart();
	void toggleEditor(AppState& s);
	void toggleAppState(AppState& s);
	void togglePause(AppState& s);

	static void onWindowResized(GLFWwindow* window, int width, int height) {
		if (width == 0 || height == 0) return;
		WINDOW.resize();
		ApplicationSystem* app = reinterpret_cast<ApplicationSystem*>(glfwGetWindowUserPointer(window));
		auto& renderer = app->rs;
		auto& sce = renderer->getSwapChainExtent();
		renderer->recreateSwapChain();
		app->ui->resize(sce.width, sce.height, app->rs->getFrameBuffers());
	}
};
