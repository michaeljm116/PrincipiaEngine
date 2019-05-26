#pragma once
#include "../pch.h"

/*
NINTENDO SWITCH CONTROLLER :
directional buttons = 16 - 19
b / v = 1
a / > = 3
y / < = 0
x / ^ = 2

+ / -= 8

13 = HOME BUTTON
*/

/*
0-3 = MAIN BUTTONS
4 = PAUSE
5 RIGHT SHOULDER
6 = RIGHT BACK
7 = LEFT SHOLDER
8 = LEFT BACK
9 = home button = exit
and theres an axis of course
*/
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

	Button& operator= (Button& b) {
		key = b.key;
		action = b.action;
		time = b.time;

		return *this;
	}
};
#define NUM_BUTTONS 10
#define NUM_GLOBAL_BUTTONS 10
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
	ControllerComponent(int id) : index(id) {};
	ControllerComponent(ControllerComponent* cc) {
		axis = cc->axis;
		index = cc->index;
		gamepad = cc->gamepad;
		for (int i = 0; i < NUM_BUTTONS; ++i) {
			buttons[i] = cc->buttons[i];
		}
	}

	/*so you toss in a 21 and you turn it into a buttonything
	also have index so you can save
	*/
};

struct GlobalController : public artemis::Component {
	Button buttons[NUM_GLOBAL_BUTTONS];
	glm::vec3 axis;

	GlobalController() {};
};
