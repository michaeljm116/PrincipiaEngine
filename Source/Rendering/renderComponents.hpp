#pragma once
#ifndef RENDERCOMPONENTS_HPP
#define RENDERCOMPONENTS_HPP

#include "../pch.h"

//enum class ObjectType {
//	Sphere,
//	Box,
//	Plane,
//	Cylinder,
//	Cone,
//	Mesh,
//	Light,
//	Camera,
//	None
//};

enum RenderType {
	RENDER_NONE = 0x00,
	RENDER_MATERIAL = 0x01,
	RENDER_PRIMITIVE = 0x02,
	RENDER_LIGHT = 0x04,
	RENDER_GUI = 0x08,
	RENDER_GUINUM = 0x10,
	RENDER_CAMERA = 0x20
};

struct RenderComponent : artemis::Component {
	RenderType type;
	RenderComponent() {};
	RenderComponent(RenderType t) : type(t) {};
};

struct SphereComponent : artemis::Component{
	float radius;
	int sphereIndex;

	SphereComponent() {};
	SphereComponent(float r) { radius = r; };
};

struct BoxComponent : artemis::Component {
	glm::vec3 center;
	glm::vec3 extents;
	int boxIndex;

	BoxComponent() {};
	BoxComponent(glm::vec3 c, glm::vec3 e) : center(c), extents(e) {};
};

struct CylinderComponent : artemis::Component {
	glm::vec3 top;
	glm::vec3 bottom;
	float radius;
	int cylinderIndex;

	CylinderComponent() {};
	CylinderComponent(glm::vec3 t, glm::vec3 b, float r) : top(t), bottom(b), radius(r) {};
};

struct  PlaneComponent : artemis::Component{
	glm::vec3 normal;
	float distance;
	int planeIndex;

	PlaneComponent() {};
	PlaneComponent(glm::vec3 n, float d) :normal(n), distance(d) {};
};

struct MeshComponent : artemis::Component {
	int meshIndex;
	int meshModelID;
	int meshResourceIndex;
	int uniqueID;
	
	MeshComponent() {};
	MeshComponent(int si) { meshIndex = si; };
	MeshComponent(int id, int ri) : meshModelID(id), meshResourceIndex(ri) {};
};

struct PrimitiveComponent : artemis::Component {
	int objIndex;	// where this object resides in the cpu @rendersystem.h...idk why its not called cpuindex
	int gpuIndex;	// where this object resides in the gpu
	int uniqueID;	// reference back to the resource manager
	glm::vec3 center;
	glm::vec3 extents;
	

	PrimitiveComponent() {};
	PrimitiveComponent(int id) : uniqueID(id) {};
};

struct ModelComponent : artemis::Component {
	int modelIndex;
	int modelUniqueID;

	ModelComponent() {};
	ModelComponent(int n) { modelUniqueID = n; };
	ModelComponent(int n, int id) { modelIndex = n; modelUniqueID = id; };
};

struct GUIComponent : artemis::Component {
	glm::vec2 min;
	glm::vec2 extents;
	glm::vec2 alignMin;
	glm::vec2 alignExt;
	int layer;
	int id;
	int ref;
	bool visible;
	GUIComponent() {};
	GUIComponent(glm::vec2 m, glm::vec2 e, glm::vec2 amin, glm::vec2 ae, int l, int i, bool v) :
		min(m), extents(e), alignMin(amin), alignExt(ae), layer(l), id(i), visible(v) {};
};
struct GUINumberComponent : GUIComponent {
	int number;
	std::vector<int> shaderReferences;
	GUINumberComponent() {};
	GUINumberComponent(glm::vec2 m, glm::vec2 e, int n){ min = m; extents = e; number = n; alignMin = glm::vec2(0.0f, 0.0f); alignExt = glm::vec2(0.1f, 1.f); layer = 0; id = 0; };
};


#endif // !GEOMETRY