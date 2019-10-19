#pragma once

#include "applicationComponents.h"
#include "../Rendering/Components/renderComponents.hpp"
#include "controllerComponent.hpp"

class PauseSystem : public artemis::EntityProcessingSystem {
private:
	artemis::ComponentMapper<PauseComponent> pauseMapper;
	artemis::ComponentMapper<GUIComponent> guiMapper;
	artemis::ComponentMapper<ControllerComponent> controllerMapper;
public:
	PauseSystem();
	void initialize();
	void processEntity(artemis::Entity &e);
	void added(artemis::Entity &e);
	void removed(artemis::Entity &e);
};