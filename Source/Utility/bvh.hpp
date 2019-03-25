/* BVH Class Copyright (C) by Mike Murrell 2019

notes: Basically what you want is a bvh that's organized into quads
quads is perfect for this engine since its focused on top-down games
so you only have to worry about the X & Z coordinates
The way it works is:
- you have an array of objects
- sort them by their X value by....
	- finding the min and max X and dividing by 2
	- BST tree where the initial value is the center
- DONT FORGET THE EXTENTS, be calcing the extents during all this as well
- then do an InOrder Tree Traversal to get a sorted list
- then you have 2 arrays of objects
- sort them by their Y value

- Then finish by having 4 trees of objects
	- if there's more than 4 objects in a quadrant, do the tree thing again
	
- finally to convert it into an array do a breadth first search, recording the indexes as you go
*/

#pragma once
#include "../Rendering/shaderStructures.hpp"
#include "../Game/geometryComponents.hpp"
#include "helpers.h"

static const int MAX_BVH_OBJECTS = 4;

enum class TreeType {
	Recursive,
	HLBVH
};
enum class SplitMethod {
	Middle, SAH, EqualsCounts
};

struct BVHNode {
	glm::vec3 bounds;
	BVHNode* children[2];
	int splitAxis;
	int firstPrimOffset;
	int nPrims;

	void initLeaf(int first, int n, const glm::vec3& b) {
		firstPrimOffset = first;
		nPrims = n;
		bounds = b;
		children[0] = children[1] = nullptr;
	}

	void initInterior(int axis, BVHNode *c0, BVHNode *c1) {
		children[0] = c0;
		children[1] = c1;
		//bounds = c0->bounds.union2D(c1->bounds);
		splitAxis = axis;
		nPrims = 0;
	}
};

struct BVHTree {
	BVHNode* root;
	int totalNodes = 0;
	std::vector<ObjectComponent> orderedPrims;
};


//typedef bob(std::vector<ObjectComponent*>)
class bvh {

public: 
	glm::vec2 center;					 //8bytes
	glm::vec2 extents;					 //8bytes

	bvh*	  quadrants[4];				 //16bytes
	std::vector<ObjectComponent*> comps; //16bytes

	bool	  isLeaf;					 //4bytes
	int		  numObjs = 0;				 //4bytes
	glm::vec2 pad;						 //8bytes
	//64bytes total :)

	bvh* build(const std::vector<ObjectComponent*> objects) {
		if (objects.size() <= MAX_BVH_OBJECTS) {
			isLeaf = true;
			if (objects.size() > 0) {
				for (int i = 0; i < objects.size(); ++i) {
					numObjs++;
					comps.push_back(objects[i]);
				}
			}
			return this;
		}
		//find max X and extents X @same time
		float cMaxX = -FLT_MAX;  float eMaxX = -FLT_MAX;
		float cMinX = FLT_MAX;  float eMinX = FLT_MAX;

		float cMaxY = -FLT_MAX;  float eMaxY = -FLT_MAX;
		float cMinY = FLT_MAX;  float eMinY = FLT_MAX;

	
		//for (itr = objects.begin(); itr != objects.end(); itr++) {
		//	itr->center.x;
		//	if (itr->center.x > cMaxX) cMaxX = itr->center.x;
		//	if (itr->center.x < cMinX) cMinX = itr->center.x;
		//	if (itr->extents.x > eMaxX) eMaxX = itr->extents.x;
		//	if (itr->extents.x < eMinX) eMinX = itr->extents.x;
		//
		//	if (itr->center.y > cMaxX) cMaxX = itr->center.y;
		//	if (itr->center.y < cMinX) cMinX = itr->center.y;
		//	if (itr->extents.y > eMaxX) eMaxX = itr->extents.y;
		//	if (itr->extents.y < eMinX) eMinX = itr->extents.y;
		//}

		for (int i = 0; i < objects.size(); ++i) {
			if (objects[i]->center.x > cMaxX) cMaxX = objects[i]->center.x;
			if (objects[i]->center.x < cMinX) cMinX = objects[i]->center.x;
			if (objects[i]->extents.x > eMaxX) eMaxX = objects[i]->extents.x;
			if (objects[i]->extents.x < eMinX) eMinX = objects[i]->extents.x;
			
			if (objects[i]->center.y > cMaxY) cMaxY = objects[i]->center.y;
			if (objects[i]->center.y < cMinY) cMinY = objects[i]->center.y;
			if (objects[i]->extents.y > eMaxY) eMaxY = objects[i]->extents.y;
			if (objects[i]->extents.y < eMinY) eMinY = objects[i]->extents.y;
		}
		
		//Calc the halfextent
		center.x = cMinX + ((cMaxX - cMinX) * 0.5f);
		extents.x = (eMaxX - eMinX) * 0.5f;
		center.y = cMinY + ((cMaxY - cMinY) * 0.5f);
		extents.y = (eMaxY - eMinY) * 0.5f;
		
		//Construct a tree
		std::vector<ObjectComponent*> left;
		std::vector<ObjectComponent*> right;
		for (int i = 0; i < objects.size(); ++i) {
			objects[i]->center.x <= center.x ? left.push_back(objects[i]) : right.push_back(objects[i]);
		}

		//construct the quads
		std::vector<ObjectComponent*> leftUp;
		std::vector<ObjectComponent*> leftDown;
		for (int i = 0; i < left.size(); ++i) {
			left[i]->center.y > center.y ? leftUp.push_back(left[i]) : leftDown.push_back(left[i]);
		}

		//construct the quads
		std::vector<ObjectComponent*> rightUp;
		std::vector<ObjectComponent*> rightDown;
		for (int i = 0; i < right.size(); ++i) {
			right[i]->center.y > center.y ? rightUp.push_back(right[i]) : rightDown.push_back(right[i]);
		}

		quadrants[0] = build(leftUp);
		quadrants[1] = build(leftDown);
		quadrants[2] = build(rightUp);
		quadrants[3] = build(rightDown);

		return this;
	}

	void insert(ObjectComponent* object) {
		//this is the end case for the leaf
		if (isLeaf) {
			if (numObjs >= MAX_BVH_OBJECTS) {
				//redo tree
				comps.push_back(object);
				*this = *build(comps);
				comps.clear();
				isLeaf = false;
			}
			else {
				comps.push_back(object);
				numObjs++;
				return;
			}
		}
		//if its not a leaf then just continue down the tree
		else {
			if (object->center.x < center.x) {
				if (object->center.y < center.y)
					quadrants[1]->insert(object);
				else
					quadrants[0]->insert(object);
			}
			else {
				if (object->center.y < center.y)
					quadrants[3]->insert(object);
				else
					quadrants[2]->insert(object);
			}
		}
	}

	bvh() {
		center = glm::vec2(0, 0);
		extents = glm::vec2(0, 0);
		isLeaf = false;
	};
	~bvh() {
		for (int i = 0; i < 4; ++i) {
			if(quadrants[i] != nullptr)
				delete quadrants[i];
		}
	};
private:
	//find center and extents/ so you're gven an array of objects and you fin
	//void findCenterExtents(glm::vec2& center, glm::vec2& extents, const std::vector<ssObject>& objects) {
	//	glm::vec2 max
	//}
	
};

