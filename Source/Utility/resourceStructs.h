/* Copyright (C) Mike Murrell 2019 Resource structs
just a bunch of structures used for
the resource manager
*/
#pragma once
#include <array>
#include <glm/gtx/quaternion.hpp>

const float BONE_EPSILON = 0.05f;
struct rJointData {
	std::array<int, 4> id;
	std::array<float, 4> weights;

	rJointData() {};
	void average() {
		float total = 0;
		for (int i = 0; i < 4; ++i) {
			if (weights[i] < BONE_EPSILON)
				weights[i] = 0;
			total += weights[i];
		}
		assert(total != 0);
		for (int i = 0; i < 4; ++i) {
			weights[i] /= total;
		}
	}
};

struct  rMaterial
{
	glm::vec3 diffuse;
	float reflective;
	float roughness;
	float transparency;
	float refractiveIndex;
	bool hasTexture;
	int uniqueID = 0;
	std::string Texture;
	std::string name;
	ssMaterial* renderedMat;

	rMaterial() {};
	rMaterial(std::string n, glm::vec3 d, float rfl, float rgh, float trns, float rfr) {

		name = n;			diffuse = d; 
		reflective = rfl;	refractiveIndex = rfr;
		roughness = rgh;	transparency = trns;

		uniqueID = name[0];
		for (int i = 1; i < name.size(); ++i) {
			uniqueID *= name[i] + name[i - 1];
		}
	}
};
struct rMesh {
	std::vector<glm::vec3> verts;
	std::vector<glm::ivec3> tris;
	std::vector<rJointData> bones;

	glm::vec3 center;
	glm::vec3 extents;
	std::string name;
	rMaterial mat;
	int matId = 0;
};
struct rModel {
	std::string name;
	std::vector<rMesh> meshes;
	glm::vec3 center;
	glm::vec3 extents;
	int uniqueID;
	int skeletonID;
};

struct rJoint {
	std::string name;
	int parentIndex;
	//glm::mat4 local_invBindPose;
	glm::mat4 invBindPose;
	glm::mat4 transform;
	rJoint() {};
};

struct rAnimKey {
	float time;
	glm::vec3 pos;
	glm::quat rot;
	glm::vec3 sca;
};

struct rAnimChannel {
	std::vector<rAnimKey> keys;
};

struct rAnimation {
	int skeletonID;
	int numChannels;
	float duration;
	float sps; //Samples per second
	std::string name;
	std::vector<rAnimChannel> channels;
};

struct rSkeleton {
	int id;
	std::vector<rJoint> joints;
	std::string name;
	std::vector<rAnimation> animations;
	glm::mat4 globalInverseTransform;

	//Joint 
	void buildGlobalBinds() {
		for (int j = 0; j < joints.size(); ++j) {
			//First build the joints
			int pi = joints[j].parentIndex;
			if (pi > 0) {
				//joints[j].global_invBindPose = joints[j].globlocalal_invBindPose;
				joints[j].invBindPose = joints[pi].invBindPose * joints[j].invBindPose;
			}
			/*
			else {
				//first build the joints
				//joints[j].global_invBindPose = joints[pi].global_invBindPose * joints[j].globasdfal_invBindPose;
				
				//then build the channels
				for (int a = 0; a < animations.size(); ++a) {
					rAnimChannel chan = animations[a].channels[j];
					for (int k = 0; k < chan.keys.size(); ++k) {
						//glm::vec3 position = chan.keys[k].pos;
						//glm::quat rotation = chan.keys[k].rot;
						//glm::vec3 scale = chan.keys[k].sca;

						chan.keys[k].pos = chan.keys[pi].pos + chan.keys[k].pos;	
						chan.keys[k].rot = chan.keys[pi].rot * chan.keys[k].rot;
						chan.keys[k].sca = chan.keys[pi].sca * chan.keys[k].sca;
					}
				}
			}*/

		}
	};

};