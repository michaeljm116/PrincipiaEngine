#pragma once
/* primitives Copyright (C) by Mike Murrell 
im about 89.4% sure this isn't used like at all anymore

*/
#ifndef PRIMITIVE_HPP
#define PRIMITIVE_HPP
#endif // !PRIMITIVE

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

#include <glm\glm.hpp> 
#include <vector>
#include "Artemis\Artemis.h"

// SSBO sphere declaration 
struct Sphere {									// Shader uses std140 layout (so we only use vec4 instead of vec3)
	glm::vec3 pos;
	float radius;
	glm::vec3 diffuse;
	float specular;
	int id;								// Id used to identify sphere for raytracing
	glm::ivec3 _pad;
};

// SSBO plane declaration
struct Plane {
	glm::vec3 normal;
	float distance;
	glm::vec3 diffuse;
	float specular;
	int id;
	glm::ivec3 _pad;
};

struct Box {
	glm::vec3 center;	//12bytes
	int _pad;			//4bytes
	glm::vec3 extents;	//12bytes
	float specular;		//4bytes
	glm::vec3 diffuse;	//12bytes
	int id;				//4bytes
}; //Total = 48 bytes

struct Mesh {
	glm::vec3 diffuse;	//12
	float specular;		//16
	glm::vec3 center;	//28
	int startIndex;		//32
	glm::vec3 extents;	//44
	int endIndex;		//48
	int id;				//52
	int startVert;		//56
	int endVert;		//60
	int pad;			//64
};
struct Triangle {
	glm::vec3 v[3];
	glm::vec3* nodeCenter;
	Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2) { v[0] = v0; v[1] = v1; v[2] = v2; };
};
struct TriangleVert {
	glm::vec3 pos;
	int _pad;
	TriangleVert() {};
	TriangleVert(glm::vec3 p) : pos(p) {};
};
struct TriangleIndex {
	glm::ivec3 v;	//12bytes
	int id;			//4bytes
	TriangleIndex() {};
	TriangleIndex(int v0, int v1, int v2, int id) : id(id) { v[0] = v0; v[1] = v1; v[2] = v2; };
	//TriangleIndex(int v0, int v1, int v2, int id) : v0(v0), v1(v1), v2(v2), id(id) {};
}; //Total = 16 bytes

struct AABB : public artemis::Component {
	glm::vec3 center;		//12bytes
	glm::vec3 extents;		//12bytes
	glm::vec3* nodeCenter;	//4bytes??
							//needs 4 more

	AABB() { center = glm::vec3(0.f); extents = glm::vec3(0.f); };
	AABB(glm::vec3 c, glm::vec3 e) : extents(e) { center = c; };

	//see if a smaller AABB is inside this aabb
	//psuedocode: if (Max.X < B.Min.X || Min.X > B.Max.X) return false;
	bool Contains(AABB b) {
		for (int i = 0; i < 3; ++i) {
			if (center[i] + extents[i] < b.center[i] - b.extents[i] || center[i] - extents[i] > b.center[i] + b.extents[i]) 
				return false;
		}
		return true;
	};
	bool Contains(AABB* b) {
		for (int i = 0; i < 3; ++i) {
			if (center[i] + extents[i] < b->center[i] - b->extents[i] || center[i] - extents[i] > b->center[i] + b->extents[i]) return false;
			if (center[i] + extents[i] < b->center[i] + b->extents[i] || center[i] - extents[i] > b->center[i] - b->extents[i]) return false;
		}
		return true;
	};
	
	//Find out if theres a triangle in it y0
	bool Contains(const TriangleIndex& ind, const std::vector<TriangleVert>& verts) {
		glm::vec3 max = center + extents;
		glm::vec3 min = center - extents;

		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; j++) {
				if (max[i] < verts[ind.v[j]].pos[i] || min[i] > verts[ind.v[j]].pos[i]) return false;
			}
		}
		return true;
	}
	
	bool Contains(const Triangle& t) {
		glm::vec3 max = center + extents;
		glm::vec3 min = center - extents;

		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j)
				if (max[i] < t.v[i][j] || min[i] > t.v[i][j]) return false;
		}
		return true;
	}
	bool Contains(const Triangle* t) {
		glm::vec3 max = center + extents;
		glm::vec3 min = center - extents;

		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j)
				if (max[i] < t->v[i][j] || min[i] > t->v[i][j]) 
					return false;
		}
		return true;
	}
};
struct BB2D : public artemis::Component{
	glm::vec2 center;
	glm::vec2 extents;
	glm::vec2* nodeCenter;
	BB2D() { center = glm::vec2(0.f); extents = glm::vec2(0.f); };
	BB2D(glm::vec2 c, glm::vec2 e) { center = c; extents = e; };

	bool Contains(const Triangle* t) {
		glm::vec2 max = center + extents;
		glm::vec2 min = center - extents;
		for (int i = 0; i < 3; ++i) {
			if (max[0] < t->v[i][0] || min[0] > t->v[i][0])
				return false;
			if (max[1] < t->v[i][2] || min[1] > t->v[i][2])
				return false;
		}
	}
	bool Contains(const Triangle& t) {
		glm::vec2 max = center + extents;
		glm::vec2 min = center - extents;
		for (int i = 0; i < 3; ++i) {
			if (max[0] < t.v[i][0] || min[0] > t.v[i][0])
				return false;
			if (max[1] < t.v[i][2] || min[1] > t.v[i][2])
				return false;
		}
	}
};
/*
enum PrimitiveType
{
	PRIM_BOX, PRIM_PLANE, PRIM_MESH, PRIM_SPHERE
};

struct PrimitiveComponent : public artemis::Component
{
	PrimitiveComponent(){};
	~PrimitiveComponent() {};

	glm::vec3 diffuse;				//12bytes
	float specular;					//4bytes
	int id;							//4bytes
	PrimitiveType type;				//4bytes		28 total
};

struct Sphere : public PrimitiveComponent {
	Sphere() { type = PRIM_SPHERE; };
	Sphere(float r, glm::vec3 pos, glm::vec3 diff, float spec, int id) 
		: radius(r), pos(pos){
		diffuse = diff; 
		specular = spec; 
		this->id = id;
		type = PRIM_SPHERE;
	};

	float radius;		//4bytes	
	glm::vec3 pos;		//12bytes
};

struct Box : public PrimitiveComponent {
	Box() { type = PRIM_BOX; };
	Box(glm::vec3 c, glm::vec3 e, glm::vec3 diff, float spec, int id) 
	:center(c), extents(e){
		diffuse = diff;
		specular = spec;
		this->id = id;
		type = PRIM_BOX;
	}
	glm::vec3 center;		//12 bytes
	glm::vec3 extents;		//12 bytes	
};

struct Plane : public PrimitiveComponent {
	glm::vec3 normal;		//12 bytes
	float distance;			//4 bytes
};

struct Mesh : public PrimitiveComponent {
	glm::vec3 center;		//12 bytes
	glm::vec3 extents;		//12 bytes
	int startIndex;			//4 bytes
	int endIndex;			//4 bytes
};
*/