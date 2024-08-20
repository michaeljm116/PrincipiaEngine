#pragma once
#include <Artemis/Component.h>
#include <glm/glm.hpp>

namespace Principia {
	struct LightComponent : public artemis::Component {
		glm::vec3 color = glm::vec3(0);
		float intensity = 0.f;
		int id = 0;

		LightComponent() {};
		LightComponent(glm::vec3 c, float i, int id) : color(c), intensity(i), id(id) {};
		//LightComponent(glm::vec3 c, glm::vec3 p, float i) : color(c), pos(p), intensity(i) {};
	};
}