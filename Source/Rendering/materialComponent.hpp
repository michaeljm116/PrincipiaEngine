#pragma once
#ifndef MATERIALCOMPONENT_HPP
#define MATERIALCOMPONENT_HPP

#include <Artemis/Component.h>

namespace Principia {
	struct MaterialComponent : public artemis::Component {
		//glm::vec3*  diffuse; 
		//float*		roughness;
		//float*		metallic;

		/*
		glm::vec3	diffuse;
		float		reflective;
		float		roughness;
		float		transparency;
		float		refractiveIndex;*/

		int			matID = 0;
		int			matUnqiueID = 0;

		MaterialComponent() {};
		MaterialComponent(int i) { matID = i; };
		MaterialComponent(int i, int ui) { matID = i; matUnqiueID = ui; };
		//MaterialComponent(glm::vec3 diff, float ref, float rough, float trans, float ri, int i, int ui) :
		//	diffuse(diff), reflective(ref), roughness(rough), transparency(trans), refractiveIndex(ri), matID(i), matUnqiueID(ui) {};
		//MaterialComponent(glm::vec3* d, float* r, float* m) : diffuse(d) { roughness = r; metallic = m; };

	};
}
#endif // !MATERIALCOMPONENT_HPP
