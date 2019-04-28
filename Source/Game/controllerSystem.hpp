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
	ControllerSystem(){ addComponentType<ControllerComponent>(); }
	~ControllerSystem() {};

	void initialize()
	{
		controllerMapper.init(*world);
	}
	void processEntity(artemis::Entity & e)
	{
		ControllerComponent* controller = controllerMapper.get(e);
		glm::vec2 tempAxis;
		if (controller->gamepad) {
			int axesCount;
			const float *axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);

			int buttonCount;
			const unsigned char *buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);

			std::vector<float> axii;
			std::vector<int> buttii;
			for (int a = 0; a < axesCount; ++a) {
				axii.push_back(axes[a]);
			}
			for (int b = 0; b < buttonCount; ++b) {
				buttii.push_back(buttons[b]);
			}

			for (int i = 0; i < 2; ++i) {
				//if (axes[i] == GLFW_PRESS) {
				//	if (i > 2)
				//		tempAxis[i] += 1;
				//	else
				//		tempAxis[i] -= 1;
				//}
				tempAxis[i] = axes[i];
			}
		}
		else {
			for (int i = 0; i < NUM_BUTTONS; ++i) {
				int bob[348];
				for (int i = 0; i < 348; ++i) {
					bob[i] = INPUT.keys[i];
				}
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
		}
		controller->axis = tempAxis;
	}
};
