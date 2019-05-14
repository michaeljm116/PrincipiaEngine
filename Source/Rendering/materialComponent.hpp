#pragma once
#ifndef MATERIALCOMPONENT_HPP
#define MATERIALCOMPONENT_HPP

#include "../pch.h"

struct MaterialComponent : public artemis::Component{
	//glm::vec3*  diffuse; 
	//float*		roughness;
	//float*		metallic;
	int			matID;
	int			matUnqiueID;

	MaterialComponent() {};
	MaterialComponent(int i) { matID = i; };
	MaterialComponent(int i, int ui) { matID = i; matUnqiueID = ui; };
	//MaterialComponent(glm::vec3* d, float* r, float* m) : diffuse(d) { roughness = r; metallic = m; };

};

#endif // !MATERIALCOMPONENT_HPP
