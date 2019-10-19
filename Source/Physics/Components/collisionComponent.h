#pragma once
#include "Artemis/Component.h"
#include <glm/glm.hpp>

namespace Principia {
	enum class CollisionState {
		None, Start, Repeat, Exit
	};
	
	struct CollisionComponent : artemis::Component {
		CollisionState state;
		int collider;
		float radius;
		glm::vec3 position;
		CollisionComponent(glm::vec3 pos, float r) : position(pos), radius(r) { state = CollisionState::None, collider = 0; }
	};
}