#pragma once
#include <Artemis/Component.h>

struct EnemyComponent : public artemis::Component {
	int health;
	float coolDown = 0.f;
	float maxCoolDown = 2.f;
	float maxBounds = 3.f;
};