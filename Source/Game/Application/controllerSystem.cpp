#include "controllerSystem.h"
#include "applicationSystem.h"
#include "../pch.h"

ControllerSystem::ControllerSystem() { 
	addComponentType<ControllerComponent>(); 
}

void ControllerSystem::initialize()
{
	controllerMapper.init(*world);
}

void ControllerSystem::begin() {
	//check for an editor switch this is a hack-job right now
	if (INPUT.keys[GLFW_KEY_LEFT_BRACKET] == GLFW_PRESS) {
		ApplicationSystem* as = (ApplicationSystem*)world->getSystemManager()->getSystem<ApplicationSystem>();
		ApplicationComponent* ac = (ApplicationComponent*)world->getSingleton()->getComponent<ApplicationComponent>();

		as->toggleEditor(ac->state);
	}
}

void ControllerSystem::processEntity(artemis::Entity & e)
{
	ControllerComponent* controller = controllerMapper.get(e);
	glm::vec2 tempAxis;
	if (controller->gamepad) {
		int buttonCount;
		const unsigned char *buttons = glfwGetJoystickButtons(controller->index, &buttonCount);
		assert(buttonCount > 0);
		//Pause button
		controller->buttons[4].action += buttons[8];
		if (buttons[8] >= GLFW_PRESS)
			controller->buttons[4].time += world->getDelta();
		else {
			controller->buttons[4].time = 0.f;
			controller->buttons[4].action = 0;
		}
		//Home Button
		controller->buttons[9].action = buttons[13];
		//if (buttons[8] >= GLFW_PRESS)
		//	controller->buttons[9].time += world->getDelta();
		//else {
		//	controller->buttons[9].time = 0.f;
		//	controller->buttons[9].action = 0;
		//}

		//other buttons
		int bs[4];
		for (int i = 0; i < 4; ++i) {
			//First handle the 4 main face buttons
			controller->buttons[i].action += buttons[i];
			if (buttons[i] >= GLFW_PRESS)
				controller->buttons[i].time += world->getDelta();
			else {
				controller->buttons[i].time = 0.f;
				controller->buttons[i].action = 0;
			}
			//These are the directional buttons
			bs[i] = buttons[i + 16];
			if (buttons[i + 16] >= GLFW_PRESS) {
				if (i < 2) {
					//controller->buttons[i].time < 1.f ? tempAxis[i] -= controller->buttons[i].time : tempAxis[i] -= 1.f;
					tempAxis[i] -= 1.f;
				}
				else if (i < 4) {
					//controller->buttons[i].time < 1.f ? tempAxis[i-2] += controller->buttons[i].time : tempAxis[i-2] += 1.f;
					tempAxis[i - 2] += 1.f;
				}
			}
		}
	}
	else {
		for (int i = 0; i < NUM_BUTTONS; ++i) {
			int action = INPUT.keys[controller->buttons[i].key];
			controller->buttons[i].action += action;
			if (action >= GLFW_PRESS) {
				controller->buttons[i].time += INPUT.deltaTime;
				if (i < 2)
					tempAxis[i] += 1;
				else if (i < 4)
					tempAxis[i - 2] -= 1;
			}
			else if (action == GLFW_RELEASE) {
				controller->buttons[i].action = 0;
				controller->buttons[i].time = 0.f;
			}
		}
	}
	//if (controller->index == 2) tempAxis = -tempAxis;
	controller->axis = tempAxis;
}