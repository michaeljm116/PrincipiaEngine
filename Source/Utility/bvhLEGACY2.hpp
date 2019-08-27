/* BVH Class Copyright (C) by Mike Murrell 2019
big ups to http://www.pbr-book.org/3ed-2018/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies.html#fig:bvh-split-middle

Question, is there any fundamental difference betwween a TLAS and a BLAS?
only thing i can think of is... a BLAS probably doesn't update much if at all
the only time a BLAS would update is IF its an animation
...right?

question is where the data comes from, is there gonna be an organized or disorganized list of data
another question is if you should do a quadBVH or just a regular BVH


MAJOR QUESITON: what is gonna be the relationship between a BVH and ECS? 
because ultimately, both the physics and the rendering would use the bvh 
esp physics with a top level bvh so like should there be a top level bvh 
graph thing thats global or.... just have like a top level bvh system bu
t how do you like... make it so that... each entity could have a bvh node
component? although technically its a bvh leaf component so there's like 
a system that takes the structure well that will couple it too much like
how do you do it so that.... okay so both the render and the physics sys
will have access to a bvh right... but you dont wanna make it necessary 
for them to talk to each other, despite both sharing the same exact bvh
what if you create singleton entity that how bout you 


TODO, right now the center = xy it should be xz. 
since youre not using center for anything else really you should just make it a vec2

*/

#pragma once
#include "../Rendering/shaderStructures.hpp"
#include "../Rendering/renderComponents.hpp"
#include "helpers.h"
#include "bvhSystem.h"


//This is what will eventually end up in the GPU, a bvh flattened to an array
//Preordered DFS, Binary with the parent having an index to the 2nd child
struct ssBVHNode {
	glm::vec2 center;	//8bytes
	glm::vec2 extents;	//8bytes
	union {
		int primOffset; 
		int secChild;   
	};					//4bytes
	int numPrims;		//4bytes
	int axis;			//4bytes
	int _pad;			//4bytes
};	//32bytes total

struct BVHTree {
	std::shared_ptr<BVHNode> root;
	int totalNodes = 0;
	std::vector<ssPrimitive> orderedPrims;
	SplitMethod splitMethod;

	void build(SplitMethod sm, TreeType tt, std::vector<PrimitiveComponent*> &pi, std::vector<ssPrimitive> &prims) {
		splitMethod = sm;
		if (tt == TreeType::Recursive) {
			root = recursiveBuild(pi, 0, pi.size(), &totalNodes, prims, orderedPrims);
		}
	}
	std::shared_ptr<BVHNode> recursiveBuild(std::vector<PrimitiveComponent*> &primInfo, int start, int end, int* totalNodes, std::vector<ssPrimitive> &prims, std::vector<ssPrimitive> &orderedPrims) {
		*totalNodes += 1;
		std::shared_ptr<BVHNode> node(new BVHNode);
		BVHBounds bounds = computeBounds(prims, start, end);

		//Check if leaf
		int numPrims = end - start;
		if (numPrims < 3) { //create leaf
			int firstPrimOffset = orderedPrims.size();
			for (int i = start; i < end; ++i) {
				orderedPrims.push_back(prims[primInfo[i]->objIndex]);
			}
			node->initLeaf(firstPrimOffset, numPrims, bounds);
		}
		//Not a leaf, create a new node
		else {
			glm::vec2 center = computeCenter(prims, start, end);
			int axis;
			center.x > center.y ? axis = 0 : axis = 1;

			int mid;// = (start + end) / 2;
			//edgecase for if the max = min?

			switch (splitMethod) {
			//This takes the center of all the centers and partitions based off who is greater/less than
			case SplitMethod::Middle:{
				float pmid = center[axis];
				PrimitiveComponent** midPtr = std::partition(&primInfo[start], &primInfo[end - 1] + 1, [axis, pmid](const PrimitiveComponent* pi){
					return pi->center[axis] < pmid;
				});
				mid = midPtr - &primInfo[0];
				if (mid != start && mid != end)
					break;
			}

			//This evenly partitions it in half
			case SplitMethod::EqualsCounts:{
				mid = (start + end) / 2;
				std::nth_element(&primInfo[start], &primInfo[mid], &primInfo[end - 1] + 1, [axis](const PrimitiveComponent* a, const PrimitiveComponent* b) {
					return a->center[axis] < b->center[axis];
				});
				break;
			}
			//Note: using SAH will make it so your bvh actually doesn't suck
			case SplitMethod::SAH:
			{}
			default:
				break;
			}

			node->initInterior(axis, recursiveBuild(primInfo, start, mid, totalNodes, prims, orderedPrims),
				recursiveBuild(primInfo, mid, end, totalNodes, prims, orderedPrims));
		}

		//node->bounds = bounds;
		return node;
	}

	std::vector<ssBVHNode> flattenBVH() {};

private:
	BVHBounds computeBounds(const std::vector<ssPrimitive> &prims, int s, int e) {
		//make an aabb of the entire scene basically
		//find the minimum x and maximum x and bounds = max-min/2 = center
		glm::vec3 min(FLT_MAX);
		glm::vec3 max(-FLT_MAX);
		for (int i = s; i < e; ++i) {
			min = tulip::minV(min, glm::vec3(prims[i].world[3]) + glm::vec3(prims[i].extents));
			max = tulip::maxV(max, glm::vec3(prims[i].world[3]) + glm::vec3(prims[i].extents));
		}
		glm::vec3 c = (max + min) * 0.5f;
		glm::vec3 ex = max - c;
		
		return BVHBounds(c, ex);
	}
	glm::vec2 computeCenter(const std::vector<ssPrimitive> &prims, int s, int e) {
		glm::vec3 min(FLT_MAX);
		glm::vec3 max(-FLT_MAX);
		for (int i = s; i < e; ++i) {
			min.length();
			min = tulip::minV(min, glm::vec3(prims[i].world[3]));
			max = tulip::maxV(max, glm::vec3(prims[i].world[3]));
		}

		return glm::vec2((max - min) * 0.5f);
	}
};
