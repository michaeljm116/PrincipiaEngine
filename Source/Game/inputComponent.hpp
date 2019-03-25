#pragma once
#include <Artemis/Artemis.h>
#include <glm/glm.hpp>

enum class InputType
{
	Local,
	AI,
	Network
};

struct InputComponent : public artemis::Component {
	glm::vec3 direction;
	int32_t index;
	float button[12]; //time held
};
