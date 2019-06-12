#ifndef  TRANSFORMCOMPONENT_HPP
#define TRANSFOMRCOMPONENT_HPP
#include "../pch.h"


struct sqt {
	glm::quat rotation;
	glm::vec3 position;
	glm::vec3 scale;

private:
	float _pad[2];
}; //48bytes //can be 40 if necessary

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
		local.position = pos;
		local.scale = sca;

		world = rotationM;
		world[3] = glm::vec4(pos, 1.f);
		//global.rotation = local.rotation;
		//global.position = local.position;
		//global.scale = local.scale;
	};
};

#endif // ! TRANSFORMCOMPONENT_HPP

