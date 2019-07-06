#pragma once
#include "includes.h"

#define MAX_BONES 64
// Maximum number of bones per vertex
#define MAX_BONES_PER_VERTEX 4
// Per-vertex bone IDs and weights
struct VertexBoneData
{
	std::array<uint32_t, MAX_BONES_PER_VERTEX> IDs;
	std::array<float, MAX_BONES_PER_VERTEX> weights;

	// Ad bone weighting to vertex info
	void add(uint32_t boneID, float weight)
	{
		for (uint32_t i = 0; i < MAX_BONES_PER_VERTEX; i++)
		{
			if (weights[i] == 0.0f)
			{
				IDs[i] = boneID;
				weights[i] = weight;
				return;
			}

		}
	}
	void average() {
		float total = 0;
		for (int i = 0; i < MAX_BONES_PER_VERTEX; ++i) {
			total += weights[i];
		}
		assert(total != 0);
		for (int i = 0; i < MAX_BONES_PER_VERTEX;) {
			weights[i] /= total;
		}
	}
};

struct TriIndex {
	int v[3];
};
struct Face {
	glm::ivec4 v;
};

struct JointObject {
	int objID;
	int faceID;
	JointObject(const int& face, const int& obj) : faceID(face), objID(obj) {};
};

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
	Vertex() {};
	Vertex(glm::vec3 p) { position = p; };
	Vertex(aiVector3D p, aiVector3D n) { position = glm::vec3(p.x, p.y, p.z); normal = glm::vec3(n.x, n.y, n.z); }
	Vertex(const aiVector3D &p, const aiVector3D &n, const ai_real &u, const ai_real &v) { position = glm::vec3(p.x, p.y, p.z); normal = glm::vec3(n.x, n.y, n.z); uv = glm::vec2(u, v); }
};
struct Mesh {
	std::string name;
	std::string originalName;
	std::vector<Face> faces;
	std::vector<Vertex> vertices;
	glm::vec3 extent;
	glm::vec3 center;
	int id;
};

struct Shape {
	int type;
	std::string name;
	std::string originalName;
	glm::vec3 center;
	glm::vec3 extents;
};

struct  Joint
{
	std::string name;
	int parentIndex;
	aiMatrix4x4 invBindPose;
	aiMatrix4x4 transform;
	glm::mat4 glInvBindPose;
	glm::mat4 glTransform;
	std::vector<JointObject> jointObjs;
	glm::vec3 center;
	glm::vec3 extents;

	std::vector<Vertex> verts;
	std::vector<Face> faces;
	std::vector<int> meshIds;
	std::vector<Shape> shapes;
};

//how bout 3 vec3's a time and a trigger  or 2?
struct KeySQT {
	aiVectorKey pos;
	aiQuatKey rot;
	aiVectorKey sca;
	float time;
};

struct AnimChannel {
	std::string name;
	int numKeys;
	std::vector<KeySQT> keys;
};

struct PrincipiaAnimation {
	int skeletonID;
	int numChannels;
	std::string name;
	std::vector<AnimChannel> channels;
	float duration;
	float fps;
};

struct  PrincipiaSkeleton
{
	int uniqueID;
	int numJoints;
	std::vector<Joint> joints;
	std::string name;
	glm::mat4 globalInverseTransform;
	std::vector<PrincipiaAnimation> animations;

	glm::vec3 center;
	glm::vec3 extents;

};

struct PrincipiaModel {
	std::string name;
	std::vector<Mesh> meshes;
	std::vector<Shape> shapes;
	int uniqueID;
	int skeletonID;
	glm::vec3 center;
	glm::vec3 extents;
};

struct RJoint {
	std::vector<Face> faces;
	std::vector<int> shapes;
	std::string name;
	int parentIndex;


	aiMatrix4x4 invBindPose;
	aiMatrix4x4 transform;
	glm::mat4 glInvBindPose;
	glm::mat4 glTransform;

	glm::vec3 center;
	glm::vec3 extents;
};
struct PrincipiaSkinnedModel {
	int uniqueID;
	int skeletonID;
	std::vector<Vertex> verts;
	std::vector<RJoint> joints;
};

struct tempRJoint {
	std::string name;
	std::vector<int> verts;
};