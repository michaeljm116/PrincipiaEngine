#pragma once
#include <Artemis/Artemis.h>
#include <glm/glm.hpp>

enum class InputType
{
	Local,
	AI,
	Network
};
struct Button {
	int key;
	int action;
	double time;
};
#define NUM_BUTTONS 8
#define NUM_GLOBAL_BUTTONS 7
///ControllerComponent, NOTE: first 6 buttons are for the axis
/// goes in order of +x +y +z -x -y -z
struct ControllerComponent : public artemis::Component {

	//glm::vec3 direction;
	//int32_t index;
	//float button[12]; //time held
	Button buttons[NUM_BUTTONS];
	glm::vec2 axis;
	int index;
	bool gamepad = false;
	ControllerComponent(int id) : index(id) { if (index == 1)gamepad = false; };

	/*so you toss in a 21 and you turn it into a buttonything
	also have index so you can save
	*/
};

struct GlobalController : public artemis::Component {
	Button buttons[NUM_GLOBAL_BUTTONS];
	glm::vec3 axis;

	GlobalController() {};
};
