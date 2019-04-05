#pragma once
#ifndef GEOMETRYCOMPONENTS_HPP
#define GEOMETRYCOMPONENTS_HPP

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

#include <glm\glm.hpp> 
#include <vector>
#include "Artemis\Artemis.h"

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

#endif // !GEOMETRY