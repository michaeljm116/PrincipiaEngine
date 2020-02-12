#pragma once
#ifndef STRUCTS_H
#define STRUCTS_H

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

struct Bounds {
	glm::vec3 center;
	glm::vec3 extents;

	Bounds(const glm::vec3& c, const glm::vec3& e) : center(c), extents(e) {};
	Bounds() { center = glm::vec3(0); extents = glm::vec3(0); };
	inline glm::vec3 max() {
		return center + extents;
	}
	inline glm::vec3 min() {
		return center - extents;
	}
	inline glm::vec3 getCenter() { return center; };
	inline glm::vec3 getExtents() { return extents; };

	Bounds combine(Bounds& b) {
		//find the highest and the lowest x and y values
		
		//this is stupid and slow but i dont care cause its offline and im lazy righit now
		glm::vec3 max;
		for (int i = 0; i < 3; ++i) max[i] = std::max(this->max()[i], b.max()[i]);// std::max(this->max(), b.max());
		glm::vec3 min;// = std::min(this->min(), b.min());
		for (int i = 0; i < 3; ++i) min[i] = std::min(this->min()[i], b.min()[i]);

		//center = halfway between the two, extents = max-center
		glm::vec3 c = (max + min) * 0.5f;
		glm::vec3 e = max - c;

		return Bounds(c, e);
	}

	float Offset(glm::vec3 c, int a) const {
		float ret = (c[a] - (center[a] - extents[a])) / (extents[a] * 2);
		return ret;
	}

	float SurfaceArea() {
		glm::vec3 te = extents * 2.f;
		return 2 * (te.x * te.y + te.x * te.z + te.y * te.z);
	}
};

struct kBounds {
	//this is just an interface 
	//kBounds(Mesh* m) : mesh(m) {};
	virtual Bounds getBounds()  = 0;
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

struct Face : public kBounds {
	glm::ivec4 v;
	std::vector<Vertex> const * vertices;

	Face(std::vector<Vertex> const * verts) : vertices(verts) {};
	
	Bounds getBounds() override {
		//find dthe max and min qualities in the verts and make a bounds from it
		glm::vec3 max = vertices->at(v.x).position;
		glm::vec3 min = vertices->at(v.x).position;

		for (int i = 1; i < 4; ++i) {
			for (int j = 0; j < 3; ++j) {
				max[j] = std::max(max[j], vertices->at(i).position[j]);
				min[j] = std::min(min[j], vertices->at(i).position[j]);
			}
		}

		glm::vec3 c = (max + min) * 0.5f;
		glm::vec3 e = max - c;
		return Bounds(c, e);

	}
};

struct flatBVHNode {
	glm::vec3 upper;
	int offset;
	glm::vec3 lower;
	int numChildren;
};

struct Mesh {
	std::string name;
	std::string originalName;
	std::vector<Face> faces;
	std::vector<Vertex> vertices;
	std::vector<flatBVHNode> bvh;
	glm::vec3 extent;
	glm::vec3 center;
	glm::mat4 transform;
	uint32_t id;
};

struct Shape : public kBounds {
	int type;
	std::string name;
	std::string originalName;
	glm::vec3 center;
	glm::vec3 extents;

	Bounds getBounds() override {
		return Bounds(center, extents);
	};
};

struct  Joint
{
	std::string name;
	int parentIndex;
	aiMatrix4x4 invBindPose;
	aiMatrix4x4 transform;
	aiMatrix4x4 offset;
	glm::mat4 glInvBindPose;
	glm::mat4 glTransform;
	glm::mat4 glGlobalTransform;
	glm::mat4 glOffset;
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
	uint32_t skeletonID;
	int numChannels;
	std::string name;
	std::vector<AnimChannel> channels;
	float duration;
	float fps;
};

struct  PrincipiaSkeleton
{
	uint32_t uniqueID;
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
	uint32_t uniqueID;
	uint32_t skeletonID;
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


#endif // !STRUCTS_H