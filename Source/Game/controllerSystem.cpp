#include "ControllerSystem.h"
#pragma once

ControllerSystem::ControllerSystem()
{
	addComponentType<ControllerComponent>();
}

ControllerSystem::~ControllerSystem()
{
}

void ControllerSystem::initialize()
{
	controllerMapper.init(*world);
}

void ControllerSystem::processEntity(artemis::Entity & e)
{
	ControllerComponent* controller = controllerMapper.get(e);
	glm::vec2 tempAxis;
	for (int i = 0; i < NUM_BUTTONS; ++i) {
		int action = INPUT.keys[controller->buttons[i].key];
		controller->buttons[i].action = action;
		if (action >= GLFW_PRESS) {
			controller->buttons[i].time += INPUT.deltaTime;
			if (i < 2)
				tempAxis[i] += 1;
			else if (i < 4)
				tempAxis[i - 2] -= 1;
		}
		else if (action == GLFW_RELEASE)
			controller->buttons[i].time = 0.f;
	}
	controller->axis = tempAxis;
}
