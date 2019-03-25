#pragma once
#include <Artemis/Artemis.h>

struct CharacterComponent : public artemis::Component {
	float speed = 1.f;
	float maxSpeed = 1.f;
	float jumpSpeed = 1.f;
	float maxJumpHeight = 1.f;

	CharacterComponent() {};
	CharacterComponent(float data[4]) : speed(data[0]), maxSpeed(data[1]), jumpSpeed(data[2]), maxJumpHeight(data[3]) {};
};