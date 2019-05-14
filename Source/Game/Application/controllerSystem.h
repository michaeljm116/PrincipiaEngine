#pragma once
/* Input System Copyright (C) by Mike Murrell

*/
#include "../Utility/Input.h"
#include "../Gameplay/characterComponent.hpp"
#include "ControllerComponent.hpp"
#include "../pch.h"

class ApplicationSystem;

class ControllerSystem : public artemis::EntityProcessingSystem {
private:
	artemis::ComponentMapper<ControllerComponent> controllerMapper;
public:
	ControllerSystem();

	void initialize();
	void begin();
	void processEntity(artemis::Entity & e);
};

