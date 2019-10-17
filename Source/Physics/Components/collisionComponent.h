#pragma once
#include "Artemis/Component.h"
#include <glm/glm.hpp>

namespace Principium {
	enum class CollisionState {
		None, Start, Repeat, Exit
	};
	
	struct CollisionComponent : artemis::Component {
		CollisionState state;
		int collider;
		float radius;
		glm::vec3 position;
	};
}