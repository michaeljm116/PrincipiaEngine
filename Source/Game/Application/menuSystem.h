#pragma once

#include "applicationComponents.h"
#include "../Rendering/renderComponents.hpp"
#include "controllerComponent.hpp"

class MenuSystem : public artemis::EntityProcessingSystem {
private:
	artemis::ComponentMapper<MenuComponent> menuMapper;
	artemis::ComponentMapper<GUIComponent> guiMapper;
	artemis::ComponentMapper<ControllerComponent> controllerMapper;
public:
	MenuSystem();

	void initialize();
	void processEntity(artemis::Entity &e);
	void added(artemis::Entity &e);
	void removed(artemis::Entity &e);


};