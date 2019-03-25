#include "inputSystem.h"
#pragma once

InputSystem::InputSystem()
{
	addComponentType<InputComponent>();
}

InputSystem::~InputSystem()
{
}

void InputSystem::initialize()
{
	inputMapper.init(*world);
}

void InputSystem::processEntity(artemis::Entity & e)
{
	InputComponent* input = inputMapper.get(e);
	input->direction = INPUT.axis;
	//if (INPUT.Keys[7].state == KEY_HELD) {
	//	input->button[0] += abs(INPUT.deltaTime);
	//}
	//else
	//	input->button[0] = 0.f;
	//for (int i = 0; i < 7; ++i) {
	//	if (INPUT.Keys[i + 5].state = KEY_HELD)
	//		input->button[i] += INPUT.deltaTime;
	//}

}

void InputSystem::update()
{
}

