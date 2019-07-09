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

#include "../pch.h"

struct ssGUI {
	glm::vec2 min;
	glm::vec2 extents;
	glm::vec2 alignMin;
	glm::vec2 alignExt;
	int layer;
	int id;
	int pad = 0;
	int visible = 1;

	ssGUI() {};
	ssGUI(glm::vec2 min, glm::vec2 max, glm::vec2 aMin, glm::vec2 aExt, int l, int i) : min(min), extents(max), alignMin(aMin), alignExt(aExt), layer(l), id(i) {};
};

struct ssPrimitive {
	glm::mat4 world; //64bytes
	glm::vec3 extents; //12bytes
	int numChildren = 0; //4bytes;
	int id; //4bytes
	int matId; //4bytes
	int startIndex = 0;
	int endIndex = 0;

};//Total = 96bytes

struct ssJoint {
	glm::mat4 world;		//64bytes
	glm::vec3 extents;		//12bytes
	int id;					//4bytes

	int startIndex = 0;		//4bytes
	int endIndex = 0;		//4bytes
	int startShape = 0; 	//4bytes
	int endShape = 0;		//4bytes
}; //Total = 96bytes


struct ssVert {
	glm::vec3 pos;
	float u;
	glm::vec3 norm;
	float v;
	ssVert() {};
	ssVert(const glm::vec3 &p, const glm::vec3 &n, const float &u, const float &v) : pos(p), norm(n), u(u), v(v) { };
};

struct ssTriangleIndex {
	glm::ivec3 v;	//12bytes
	int id;			//4bytes
	ssTriangleIndex() {};
	ssTriangleIndex(glm::ivec3 v, int id) : v(v), id(id) {};
	ssTriangleIndex(int v0, int v1, int v2, int id) : id(id) { v[0] = v0; v[1] = v1; v[2] = v2; };
}; //Total = 16 bytes

struct ssIndex {
	glm::ivec4 v; //16bytes
	ssIndex() {};
	ssIndex(const glm::ivec4 &face) : v(face) {};
};

struct ssShape {
	glm::vec3 center;
	int matID = 0;
	glm::vec3 extents;
	int type;
	ssShape() {};
	ssShape(const glm::vec3& c, const glm::vec3 e, int t) : center(c), extents(e), type(t) {};
}; //Total = 16bytes

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
