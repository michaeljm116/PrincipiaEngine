#pragma once

#include <Artemis/Artemis.h>
#include <glm/gtx/quaternion.hpp>

struct Joint
{
	int parentIndex;
	glm::mat4 invBindPose;
	glm::mat4 transform;
	glm::mat4 global_Transform;
	glm::mat4 final_Transform;
	Joint() {};
};

struct Skeleton {
	int id;
	int index; //index in the resource manager
	std::vector<Joint> joints;
};

struct Channel {
	int key_current;
	int key_next;
	glm::mat4 combined;
	glm::vec3 combinedPos;
	glm::quat combinedRot;
	glm::vec3 combinedSca;

	Channel() { key_current = 0; key_next = 1; combined = glm::mat4(1.f); }
};

struct AnimationComponent : public artemis::Component {
	Skeleton skeleton;
	int animIndex;
	std::vector<Channel> channels;
	float time;
	bool loop;

	AnimationComponent(int id) {
		time = 0.f;
		loop = true;
		animIndex = 0;
		skeleton.id = id;
	}
	void refresh() {

	}
};