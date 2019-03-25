#pragma once
#include "inputComponent.hpp"

struct ButtonComponent : public artemis::Component {
	bool pressed = false;
	bool collided = false;
	ButtonComponent() {};
};

struct DoorComponent : public artemis::Component {
	bool open = false;
	DoorComponent() {};
};