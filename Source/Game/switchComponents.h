#pragma once
#include "ControllerComponent.hpp"

struct SwitchComponent : public artemis::Component {
	bool pressed = false;
	bool collided = false;
	SwitchComponent() {};
};

struct DoorComponent : public artemis::Component {
	bool open = false;
	DoorComponent() {};
};