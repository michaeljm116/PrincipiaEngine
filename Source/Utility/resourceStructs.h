/* Copyright (C) Mike Murrell 2019 Resource structs
just a bunch of structures used for
the resource manager
*/
#pragma once
#include <array>
#include <string>
#include "bvhComponent.hpp"
//#include <glm/glm.hpp>
//#include "../pch.h"

//Structure for the config file, will expand moar l8r

namespace Principia {
	struct rConfig {
		int numControllersConfigs;
		std::vector<std::array<int, 24>> controllerConfigs;
	};

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
		int textureID = -1;
		int uniqueID = 0;
		std::string Texture;
		std::string name;
		ssMaterial* renderedMat;

		rMaterial() {};
		rMaterial(std::string n, glm::vec3 d, float rfl, float rgh, float trns, float rfr, int ti) {

			name = n;			diffuse = d;
			reflective = rfl;	refractiveIndex = rfr;
			roughness = rgh;	transparency = trns;
			textureID = ti;

			uniqueID = name[0];
			for (size_t i = 1; i < name.size(); ++i) {
				uniqueID *= name[i] + name[i - 1];
			}
		}
	};
	struct rVertex {
		glm::vec3 pos;
		glm::vec3 norm;
		glm::vec2 uv;
		rVertex(const glm::vec3& p, const glm::vec3& n) : pos(p), norm(n) {};
		rVertex() {};
	};

	struct rMesh {
		std::vector<rVertex> verts;
		std::vector<glm::ivec4> faces;
		std::vector<ssBVHNode> bvh;
		std::vector<rJointData> bones;

		glm::vec3 center;
		glm::vec3 extents;
		std::string name;
		rMaterial mat;
		int matId = 0;
		int meshID;
	};

	struct rShape {
		std::string name;
		int type;
		glm::vec3 center;
		glm::vec3 extents;
	};
	struct rModel {
		std::string name;
		std::vector<rMesh> meshes;
		std::vector<rShape> shapes;
		glm::vec3 center;
		glm::vec3 extents;
		int uniqueID;
		int skeletonID;
		bool skinned = false;
	};
	struct rPose {
		std::string name;
		std::vector<std::pair<sqt, int>> pose;	
		int hashVal;
	};
	struct rPoseList {
		std::string name;
		std::vector<rPose> poses;
		int hashVal;
	};
}