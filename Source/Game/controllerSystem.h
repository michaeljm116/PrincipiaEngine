#pragma once
/* Input System Copyright (C) by Mike Murrell 
*/
#include "../Utility/Input.h"
#include "ControllerComponent.hpp"
#include <Artemis/Artemis.h>

class ControllerSystem : public artemis::EntityProcessingSystem {
private:
	artemis::ComponentMapper<ControllerComponent> controllerMapper;

public:
	ControllerSystem();
	~ControllerSystem();

	void initialize();
	void processEntity(artemis::Entity &e);
};