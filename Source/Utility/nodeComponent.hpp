#ifndef NODECOMPONENT_H_
#define NODECOMPONENT_H_

#include <Artemis\Artemis.h>
#include <list>
#include <vector>
//enum class ComponentFlag {
//	None = 0,
//	Node = 1 << 0,
//	Transform = 1 << 1,
//	Material = 1 << 2,
//	Light = 1 << 3,
//	Camera = 1 << 4,
//	Mesh = 1 << 5,
//	Subset = 1 << 6,
//	Box = 1 << 7,
//	Sphere = 1 << 8,
//	AABB = 1 << 9
//};

/*typedef unsigned char ComponentFlag;
enum class ComponentFlag {
	Node = 0x01,
	Transform = 0x02,
	Material = 0x04,
	Light = 0x08,
	Camera = 0x10,
	Mesh = 0x20,
	Subset = 0x40,
	Box = 0x80,
	//Sphere = 0xF0
};*/

/*

if (node->flags & COMPONENT_TRANSFORM) {
}
if (node->flags & COMPONENT_MATERIAL) {
}
if (node->flags & COMPONENT_LIGHT) {
}
if (node->flags & COMPONENT_CAMERA){
}
if (node->flags & COMPONENT_MESH) {
}
if (node->flags & COMPONENT_SUBSET) {
}
if (node->flags & COMPONENT_BOX) {
}
if (node->flags & COMPONENT_SPHERE) {
}
if (node->flags & COMPONENT_PLANE) {
}

*/

enum ComponentFlag {
	COMPONENT_NODE = 0x01,
	COMPONENT_TRANSFORM = 0x02,
	COMPONENT_MATERIAL = 0x04,
	COMPONENT_LIGHT = 0x08,
	COMPONENT_CAMERA = 0x10,
	COMPONENT_MODEL = 0x20,
	COMPONENT_MESH = 0x40,
	COMPONENT_BOX = 0x80,
	COMPONENT_SPHERE = 0x0100,
	COMPONENT_PLANE = 0x0200,
	COMPONENT_AABB = 0x0400,
	COMPONENT_CYLINDER = 0x0800,
	COMPONENT_SKINNED = 0x1000,
	COMPONENT_RIGIDBODY = 0x2000,
	COMPONENT_CCONTROLLER = 0x4000,
	COMPONENT_PRIMITIVE = 0x8000,
	COMPONENT_COLIDER = 0x10000,
	COMPONENT_IMPULSE = 0X20000,
	COMPONENT_SPRING = 0X40000,
	COMPONENT_BUTTON = 0x80000
};

enum Tagflags {
	TAG_NONE = 0x01,
	TAG_PLAYER = 0x02,
	TAG_WALL = 0x04,
	TAG_GOAL = 0x08
};

enum class ObjectType {
	SPHERE = -1,
	BOX = -2,
	CYLINDER = -3,
	PLANE = 4
};

//typedef unsigned long ComponentFlag;
struct NodeComponent : artemis::Component{
	artemis::Entity* data;
	NodeComponent*	 parent;
	std::vector<NodeComponent*> children;
	std::string name;
	bool clicked = false;
	bool isDynamic = false;
	unsigned int flags = COMPONENT_NODE;
	unsigned int tags = TAG_NONE;

	NodeComponent(artemis::Entity* d) : data(d) { parent = this; };
	NodeComponent(artemis::Entity* d, NodeComponent* p) : data(d), parent(p) {};
	NodeComponent(artemis::Entity* d, NodeComponent* p, NodeComponent* child) : data(d), parent(p) { children.push_back(child); };
	NodeComponent(artemis::Entity* d, NodeComponent* p, NodeComponent copy) : parent(p), data(d), name(copy.name) { flags = copy.flags; };
	NodeComponent(artemis::Entity* d, std::string n, unsigned int f) : data(d), name(n) { flags |= f; parent = nullptr; };


};


#endif // !NODECOMPONENT_H_