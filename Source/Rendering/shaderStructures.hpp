#pragma once
/* Shader Structures Copyright (C) by Mike Murrell 2017
These are all the structs that will go into the compute
shader, everything must be 16byte aligned
*/
#ifndef SHADERSTRUCTURES_HPP
#define SHADERSTRUCTURES_HPP
#endif // !PRIMITIVE

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

#include <glm\glm.hpp> 
#include <vector>
#include <chrono>
#include <iostream>
#include "Artemis\Artemis.h"

struct ssPrimitive {
	glm::mat4 world; //64bytes
	glm::vec3 center; //12 bytes
	int id; //4bytes
	glm::vec3 extents; //12bytes
	int matId; //4bytes


	//mat3 = 48
	//mat3 = 48
	//vec3 = 12
	//vec3 = 12
	//int2 = 8 = 128bytes

	//int type; //4bytes
	//instead of having a type, just have it so id starts at like 10 for meshes+
	//sphere = 0, box = 1, cyl = 2, plane = 3, (cone4,circle5)
};//Total = 96bytes

// SSBO sphere declaration 
struct ssSphere {				// Shader uses std140 layout (so we only use vec4 instead of vec3)
	glm::vec3 pos;
	float radius;
	int id;						// Id used to identify sphere for raytracing
	int matID;
	glm::ivec2 _pad;
}; //Total = 32 bytes

// SSBO plane declaration
struct ssPlane {
	glm::vec3 normal;
	float distance;
	int id;
	int matID;
	glm::ivec2 _pad;
};//Total = 32 bytes

struct ssBox {
	glm::vec3 center;	//12bytes
	int matID;			//4bytes
	glm::vec3 extents;	//12bytes
	int id;				//4bytes
	glm::mat4 world;	//64bytes
	glm::mat4 invWorld; //64bytes
}; //Total = 32 bytes + 64 bytes + 64bytes = 160bytes ewwww

struct ssCylinder {
	glm::vec3 top;		//12bytes
	int matID;			//4bytes
	glm::vec3 bottom;	//12bytes
	float radius;		//4bytes
}; //Total = 32 bytes

struct ssTriangleVert {
	glm::vec3 pos;
	int _pad;
	ssTriangleVert() {};
	ssTriangleVert(glm::vec3 p) : pos(p) {};
};

struct ssTriangleIndex {
	glm::ivec3 v;	//12bytes
	int id;			//4bytes
	ssTriangleIndex() {};
	ssTriangleIndex(glm::ivec3 v, int id) : v(v), id(id) {};
	ssTriangleIndex(int v0, int v1, int v2, int id) : id(id) { v[0] = v0; v[1] = v1; v[2] = v2; };
}; //Total = 16 bytes


struct ssMesh {
	int startIndex; //4
	int endIndex; //4
	int startVert; //4
	int endVert; //4
};

struct ssMaterial {
	glm::vec3 diffuse;
	float reflective;

	float roughness;
	float transparency;
	float refractiveIndex;
	int	  textureID;
	ssMaterial() {};
	//ssMaterial(glm::vec3 d, float m, float r) { diffuse = d, metallic = m; roughness = r; };
	ssMaterial(glm::vec3 d, float rfl, float ruf, float trn, float rfr) { diffuse = d; reflective = rfl; transparency = trn; refractiveIndex = rfr; textureID = 0; };
	//ssMaterial(glm::vec3 d, float m, float r, bool t, int id) { diffuse = d; metallic = m; roughness = r; hasTexture = b; textureID = id; };
};	//32 bytes

struct ssQuad {
	glm::vec3 v[4]; //12 * 4 = 48bytes
	int id;			//4bytes
	glm::vec3 diffuse; //12 bytes
}; //64bytes total

struct ssLight {
	glm::vec3 pos;
	float intensity;
	glm::vec3 color;
	int id;
};


struct Timer {
	std::chrono::time_point<std::chrono::steady_clock> start;
	std::chrono::time_point<std::chrono::steady_clock> end;
	std::chrono::duration<float> duration;
	const char* name;
	float ms;

	Timer() {};
	Timer(const char* n) {
		name = n;
	};
	~Timer() {};
	void Start() {
		start = std::chrono::high_resolution_clock::now();
	};
	void End() {
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;

		ms = duration.count() * 1000.0f;
	};
};
