#pragma once
#include <Artemis/Artemis.h>
#include <glm/glm.hpp>

struct CameraComponent : public artemis::Component {
	glm::vec3 lookat;
	glm::vec3 pos;
	float fov;

	CameraComponent() {};
	CameraComponent(glm::vec3 l, float f) :lookat(l), fov(f) {};
	CameraComponent(glm::vec3 l, glm::vec3 p, float f) : lookat(l), pos(p), fov(f) {};
};