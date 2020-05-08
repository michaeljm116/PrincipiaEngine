#ifndef TRANSFORMCOMPONENT_HPP
#define TRANSFOMRCOMPONENT_HPP
#pragma once
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

#ifndef GLM_FORCE_SSE2 
#define GLM_FORCE_SSE2 
#endif

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/simd/common.h>
#include <Artemis/Artemis.h>

namespace Principia {
	struct sqt {
		glm::quat rotation = glm::quat();
		glm::vec4 position = glm::vec4(0);
		glm::vec4 scale = glm::vec4(1);

	}; //48bytes //can be 40 if necessary

	inline bool operator==(const sqt& a, const sqt& b) {
		uint_fast8_t p = (a.position == b.position);
		uint_fast8_t r = (a.rotation == b.rotation) << 1;
		uint_fast8_t s = (a.scale == b.scale) << 2;

		uint_fast8_t ret = p | r | s;

		return (ret == 7);
	}
	
	struct TransformComponent : public artemis::Component {
		glm::mat4 world; //	translation rotation scale matrix
		glm::mat4 TRM;  // note: this is a Translation+Rotation Matrix, scale is left separate
		sqt local;
		sqt global;

		glm::vec3 eulerRotation;

		TransformComponent() { local.scale = glm::vec4(1.f); local.rotation = glm::quat(); local.position = glm::vec4(0); world = glm::mat4(0); };
		TransformComponent(glm::vec3 center, glm::vec3 extents) {
			*this = TransformComponent(center, glm::vec3(0.f), extents);
		}
		TransformComponent(glm::vec3 pos, glm::vec3 rot, glm::vec3 sca) : eulerRotation(rot) {


			glm::mat4 rotationM;
			rotationM = glm::rotate(rotationM, glm::radians(rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
			rotationM = glm::rotate(rotationM, glm::radians(rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
			rotationM = glm::rotate(rotationM, glm::radians(rot.z), glm::vec3(0.0f, 0.0f, 1.0f));
			local.rotation = glm::toQuat(rotationM);
			local.position = glm::vec4(pos, 0.f);
			local.scale = glm::vec4(sca, 0.f);

			world = rotationM;
			world[3] = glm::vec4(pos, 1.f);
			//global.rotation = local.rotation;
			//global.position = local.position;
			//global.scale = local.scale;
		};
	};
}

#endif // ! TRANSFORMCOMPONENT_HPP

