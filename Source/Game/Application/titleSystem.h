#pragma once

#include "applicationComponents.h"
#include "../Rendering/Components/renderComponents.hpp"
#include "controllerComponent.hpp"

class TitleSystem : public artemis::EntityProcessingSystem {
private:
	artemis::ComponentMapper<TitleComponent> titleMapper;
	artemis::ComponentMapper<GUIComponent> guiMapper;
	artemis::ComponentMapper<ControllerComponent> controllerMapper;
public:
	TitleSystem();
	void initialize();
	void processEntity(artemis::Entity &e);
	void added(artemis::Entity &e);
	void removed(artemis::Entity &e);


};