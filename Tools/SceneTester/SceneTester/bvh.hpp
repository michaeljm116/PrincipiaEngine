#pragma once
#include "transformComponent.hpp"
struct BvhNode {
	BvhNode* child1;
	BvhNode* child2;
	Bounds bounds; //24bytes
	bool isLeaf;
	std::vector<Bounds> prims;
};