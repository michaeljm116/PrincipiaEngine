#pragma once
#include "../pch.h"

struct CharacterComponent : public artemis::Component {
	float speed = 20.f;
	float maxSpeed = 1.f;
	float jumpSpeed = 1.f;
	float maxJumpHeight = 1.f;

	float hitPower = 40.f;
	bool wasHit = false;

	CharacterComponent() {};
	CharacterComponent(float data[4]) : speed(data[0]), maxSpeed(data[1]), jumpSpeed(data[2]), maxJumpHeight(data[3]) {};
};