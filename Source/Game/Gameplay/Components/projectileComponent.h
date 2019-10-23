#pragma once
#include "Artemis/Component.h"
#include <glm/glm.hpp>

struct ProjectileComponent : public artemis::Component {
	glm::vec3 dir;
	float speed = 0.75f;
	float time = 0.f;
	float maxTime = 0.5f;

	ProjectileComponent(glm::vec3 d, float s) : dir(d), speed(s) {};
};