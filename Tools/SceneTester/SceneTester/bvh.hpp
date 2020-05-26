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
#include "kdtree.hpp"
#include <algorithm>
#include <glm/glm.hpp>


static const int MAX_BVH_OBJECTS = 4;

enum class TreeType {
	Recursive,
	HLBVH
};
enum class SplitMethod {
	Middle, SAH, EqualsCounts
};


struct BVHNode {
	Bounds bounds;
	std::shared_ptr<BVHNode> children[2];
	int splitAxis;
	int firstPrimOffset;
	int nPrims;
	std::vector<Bounds> leafBounds;

	void initLeaf(std::vector<Bounds> lb, const Bounds& b, int fpo, int np) {
		firstPrimOffset = fpo;
		nPrims = np;
		bounds = b;
		children[0] = children[1] = nullptr;
		leafBounds = lb;
	}

	void initInterior(int axis, std::shared_ptr<BVHNode> c0, std::shared_ptr<BVHNode> c1) {
		children[0] = c0;
		children[1] = c1;
		//bounds = c0->bounds.union2D(c1->bounds);
		bounds = c0->bounds.combine(c1->bounds);
		splitAxis = axis;
		//nPrims = 0;
	}
};

struct FlatBVHNode {
	Bounds bounds;
	union 
	{
		int primOffset;
		int rightChildOffset;
	};
	int splitAxis;
	int numChildren;
};

struct BVHTree {
	std::shared_ptr<BVHNode> root;
	int totalNodes = 0;
	std::vector<Bounds> orderedPrims;
	SplitMethod splitMethod;
	std::vector<FlatBVHNode> flattenedBVH;

	void build(SplitMethod sm, TreeType tt, std::vector<Bounds>& prims) {
		splitMethod = sm;
		if (tt == TreeType::Recursive) {
			root = recursiveBuild(prims, &totalNodes, 0, prims.size());
		}
	}
	std::shared_ptr<BVHNode> recursiveBuild(std::vector<Bounds> &prims, int* totalNodes, int start, int end) {
		//if (prims.size() == 0) return nullptr;
		*totalNodes += 1;
		std::shared_ptr<BVHNode> node(new BVHNode);
		Bounds bounds = computeBounds(prims, start, end);

		//Check if leaf
		int numPrims = end - start; // prims.size();
		int prevOrdered = orderedPrims.size();
		if (numPrims < MAX_BVH_OBJECTS) { //create leaf
			for (int i = start; i < end; ++i) {
				orderedPrims.push_back(prims[i]);
			}
			node->initLeaf(prims, bounds, prevOrdered, numPrims);
		}
		//Not a leaf, create a new node
		else {
			Bounds centroidBounds = computeCentroidBounds(prims, start, end);
			int axis = chooseAxis(centroidBounds.extents);
			int mid = (start + end) / 2;

			//edgecase??
			if (centroidBounds.max()[axis] == centroidBounds.min()[axis]) {
				for (int i = start; i < end; ++i) {
					orderedPrims.push_back(prims[i]);
				}
				node->initLeaf(prims, bounds, prevOrdered, numPrims);
				return node;
			}

			else {
				switch (splitMethod) {
				//This takes the center of all the centers and partitions based off who is greater/less than
				case SplitMethod::Middle: {
					Bounds *midPtr = std::partition(&prims[start], &prims[end - 1] + 1, [axis, centroidBounds](const Bounds &a) {
						return a.center[axis] < centroidBounds.center[axis];
					});
					mid = midPtr - &prims[0];
					if (mid != start && mid != end)
						break;
				}

				//This evenly partitions it in half
				case SplitMethod::EqualsCounts: {
					break;
				}
				//Note: using SAH will make it so your bvh actually doesn't suck
				case SplitMethod::SAH:
				{
					if (numPrims <= 4) {
						mid = (start + end) >> 1;
						std::nth_element(&prims[start], &prims[mid], &prims[end - 1] + 1, [axis](const Bounds& a, const Bounds& b) {
							return a.center[axis] < b.center[axis];
						});
					}
					else {
						//initialize the buckets
						constexpr int numBuckets = 12;
						Bucket buckets[numBuckets];
						for (int i = start; i < end; ++i) {
							int b = numBuckets * centroidBounds.Offset(prims[i].center, axis);
							if (b == numBuckets) b--;
							buckets[b].count++;
							buckets[b].bounds.combine(prims[i]);
						}

						constexpr int nb = numBuckets - 1;
						float cost[nb];
						for (int i = 0; i < nb; ++i) {
							Bounds b0, b1;
							int c0 = 0, c1 = 0;

							for (int j = 0; j <= i; ++j) {
								b0.combine(buckets[j].bounds);
								c0 += buckets[j].count;
							}
							for (int j = i + 1; j < numBuckets; ++j) {
								b1.combine(buckets[j].bounds);
								c1 += buckets[j].count;
							}
							cost[i] = .125f + (c0 * b0.SurfaceArea() + c1 * b1.SurfaceArea()) / bounds.SurfaceArea();
						}

						float minCost = cost[0];
						int minCostSplitBucket = 0;
						for (int i = 0; i < nb; ++i) {
							if (cost[i] < minCost) {
								minCost = cost[i];
								minCostSplitBucket = i;
							}
						}
						float leafCost = numPrims;
						if (numPrims > MAX_BVH_OBJECTS || minCost < leafCost) {
							Bounds *midPtr = std::partition(&prims[start], &prims[end - 1] + 1, [centroidBounds, numBuckets, minCostSplitBucket, axis](const Bounds &a) {
								int b = (numBuckets) * centroidBounds.Offset(a.center, axis);
								if (b == numBuckets) b = numBuckets - 1;
								return b <= minCostSplitBucket;
								//return a.center[axis] < centroidBounds.center[axis];
							});
							mid = midPtr - &prims[0];
							if (mid != start && mid != end)
								break;
						}
						else { //create leaf
							for (int i = start; i < end; ++i) {
								orderedPrims.push_back(prims[i]);
							}
							node->initLeaf(prims, bounds, prevOrdered, numPrims);
							return node;
						}
					}
				}
				default:
					break;
				}
				node->initInterior(axis, recursiveBuild(prims, totalNodes, start, mid),	recursiveBuild(prims, totalNodes, mid, end));
			}
		}

		node->bounds = bounds;
		return node;
	}

	void flattenBVH() {
		int offset = 0;
		flattenedBVH.reserve(totalNodes);
		flattenBVH(root, &offset);
	}

private:

	int flattenBVH(std::shared_ptr<BVHNode> node, int* offset) {
		//first push back a node
		FlatBVHNode fbn;
		fbn.bounds = node->bounds;
		fbn.splitAxis = node->splitAxis;
		flattenedBVH.emplace_back(fbn);
		int index = flattenedBVH.size() - 1;

		//increment the offset
		*offset += 1;

		//check if leaf
		if (node->nPrims > 0) {
			flattenedBVH[index].numChildren = node->nPrims;
			flattenedBVH[index].primOffset = node->firstPrimOffset;
		}
		//else make new node
		else {
			flattenBVH(node->children[0], offset);
			flattenedBVH[index].rightChildOffset = flattenBVH(node->children[1], offset);
		}
		//return offset
		return *offset;
	}

	Bounds computeBounds(std::vector<Bounds> &prims, int start, int end) {
		//make an aabb of the entire scene basically
		//find the minimum x and maximum x and bounds = max-min/2 = center
		glm::vec3 min(FLT_MAX);
		glm::vec3 max(-FLT_MAX);

		for(int i = start; i < end; ++i){
			min = minV(min, prims[i].min());
			max = maxV(max, prims[i].max());
		}

		glm::vec3 c = (max + min) * 0.5f;
		glm::vec3 ex = max - c;

		return Bounds(c, ex);
	}

	Bounds computeCentroidBounds(std::vector<Bounds> &prims, int start, int end) {
		glm::vec3 min(FLT_MAX);
		glm::vec3 max(-FLT_MAX);

		for (int i = start; i < end; ++i) {
			min = minV(min, prims[i].center);
			max = maxV(max, prims[i].center);
		}

		glm::vec3 c = (max + min) * 0.5f;
		glm::vec3 ex = max - c;
		return Bounds(c, ex);
	}

	int chooseAxis(const glm::vec3& c) {
		if ((c.x > c.y) && (c.x > c.z)) return 0;
		else if ((c.y > c.x) && (c.y > c.z)) return 1;
		else return 2;
	}
};
