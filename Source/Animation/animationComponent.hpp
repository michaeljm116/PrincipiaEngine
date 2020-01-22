#pragma once
namespace Principia {
	struct Joint
	{
		int parentIndex;
		glm::mat4 invBindPose;
		glm::mat4 transform;
		glm::mat4 global_Transform;
		glm::mat4 final_Transform;
		Joint() {};
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

	struct JointComponent : public artemis::Component {
		glm::mat4 bindPose;
		glm::mat4 invBindPose;
		glm::mat4 transform;
		glm::mat4 global_Transform;
		glm::mat4 final_Transform;

		glm::vec3 center;
		glm::vec3 extents;
		int parentIndex;
		int uniqueID;
		int renderIndex;
		int numShapes;
		//int 
		JointComponent() {};
		JointComponent(glm::mat4 trans, glm::vec3 ext, int pI, int uID, int num) :
			bindPose(trans), extents(ext), parentIndex(pI), uniqueID(uID), numShapes(num) {};
	};


	struct Skeleton {
		int id;
		int index; //index in the resource manager
		std::vector<JointComponent*> joints;
	};

	struct ChannelComponent : public artemis::Component {
		int key_current;
		int key_next;
	};

	struct AnimationComponent : public artemis::Component {
		Skeleton skeleton;
		int animIndex;
		std::vector<Channel> channels;
		float time;
		bool loop;
		bool on = true;

		AnimationComponent(int id) {
			time = 0.f;
			loop = true;
			animIndex = 0;
			skeleton.id = id;
		}
	};
}