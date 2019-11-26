#pragma once

#include "structs.h"

namespace tulip {
	template<typename T>
	inline T max(const T& a, const T& b) {
		if (a > b) return a; else return b;
	};

	template <typename T>
	inline T min(const T& a, const T& b) {
		if (a < b) return a; else return b;
	}

	template <typename T>
	inline T maxV(const T& a, const T& b) {
		T res;
		for (int i = 0; i < a.length(); ++i) {
			a[i] > b[i] ? res[i] = a[i] : res[i] = b[i];
		}

		return res;
	}

	template <typename T>
	inline T minV(const T& a, const T& b) {
		T res;
		for (int i = 0; i < a.length(); ++i) {
			a[i] < b[i] ? res[i] = a[i] : res[i] = b[i];
		}

		return res;
	}
}

enum class TreeType {
	Recursive,
	HLBVH
};
enum class SplitMethod {
	Middle, SAH, EqualsCounts
};

#define MAX_BVH_OBJECTS 13

struct BVHBounds {
	glm::vec3 center;
	glm::vec3 extents;

	BVHBounds(glm::vec3 c, glm::vec3 e) : center(c), extents(e) {};
	BVHBounds() {};
	glm::vec3 max() {
		return center + extents;
	}
	glm::vec3 min() {
		return center - extents;
	}

	BVHBounds combine(BVHBounds b) {
		if (center.x == NAN) return b;
		//find the highest and the lowest x and y values
		glm::vec3 max = tulip::maxV(this->max(), b.max());
		glm::vec3 min = tulip::minV(this->min(), b.min());

		//center = halfway between the two, extents = max-center
		glm::vec3 c = (max + min) * 0.5f;
		glm::vec3 e = max - c;

		return BVHBounds(c, e);
	}

	BVHBounds combine(glm::vec3 c, glm::vec3 e) {
		glm::vec3 max = tulip::maxV(this->max(), (c + e));
		glm::vec3 min = tulip::minV(this->min(), (c - e));
		glm::vec3 ce = (max + min) * 0.5f;

		return BVHBounds(ce, max - ce);
	}

	float Offset(glm::vec3 c, int a) const {
		float ret = (c[a] - (center[a] - extents[a])) / (extents[a] * 2);
		return ret;
	}

	float SurfaceArea() {
		glm::vec3 te = extents * 2.f;
		return 2 * (te.x * te.y + te.x * te.z + te.y * te.z);
	}
};

struct BVHBucket {
	int count = 0;
	BVHBounds bounds;// = BVHBounds(glm::vec3(0), glm::vec3(0));
};

struct BVHNode {
	BVHBounds bounds;
	std::shared_ptr<BVHNode> children[2];
	int splitAxis;
	int firstPrimOffset;
	int nPrims;

	void initLeaf(int first, int n, const BVHBounds& b) {
		firstPrimOffset = first;
		nPrims = n;
		bounds = b;
		children[0] = children[1] = nullptr;
	}

	void initInterior(int axis, std::shared_ptr<BVHNode> c0, std::shared_ptr<BVHNode> c1) {
		children[0] = c0;
		children[1] = c1;
		//bounds = c0->bounds.union2D(c1->bounds);
		bounds = c0->bounds.combine(c1->bounds);
		splitAxis = axis;
		nPrims = 0;
	}
};



BVHBounds computeBounds(int s, int e, std::vector<Face>& prims)
{
	//make an aabb of the entire scene basically
	//find the minimum x and maximum x and bounds = max-min/2 = center
	glm::vec3 min(FLT_MAX);
	glm::vec3 max(-FLT_MAX);
	for (int i = s; i < e; ++i) {
		Bounds fb = prims[i].getBounds();

		min = tulip::minV(min, fb.min());
		max = tulip::maxV(max, fb.max());
	}
	glm::vec3 c = (max + min) * 0.5f;
	glm::vec3 ex = max - c;

	return BVHBounds(c, ex);
}

BVHBounds computeCentroidBounds(int s, int e, std::vector<Face>& prims)
{
	//make an aabb of the entire scene basically
	//find the minimum x and maximum x and bounds = max-min/2 = center
	glm::vec3 min(FLT_MAX);
	glm::vec3 max(-FLT_MAX);
	for (int i = s; i < e; ++i) {
		Bounds fb = prims[i].getBounds();
		min = tulip::minV(min, fb.min());
		max = tulip::maxV(max, fb.max());
	}
	glm::vec3 c = (max + min) * 0.5f;
	glm::vec3 ex = max - c;

	return BVHBounds(c, ex);
}

int chooseAxis(const glm::vec3& c) {
	if ((c.x > c.y) && (c.x > c.z)) return 0;
	else if ((c.y > c.x) && (c.y > c.z)) return 1;
	else return 2;
}

std::shared_ptr<BVHNode> recursiveBuild(int start, int end, int * totalNodes, std::vector<Face> &orderedPrims, std::vector<Face>& prims)
{
	*totalNodes += 1;
	std::shared_ptr<BVHNode> node(new BVHNode);// std::make_shared<BVHNode>();
	BVHBounds bounds = computeBounds(start, end, prims);

	//Check if leaf
	int numPrims = end - start;
	int prevOrdered = orderedPrims.size();
	if (numPrims < MAX_BVH_OBJECTS) { //create leaf
		for (int i = start; i < end; ++i)
			orderedPrims.emplace_back(prims[i]);
		node->initLeaf(prevOrdered, numPrims, bounds);
	}
	//Not a leaf, create a new node
	else {
		BVHBounds centroid = computeCentroidBounds(start, end, prims);
		int axis = chooseAxis(centroid.center);
		int mid = (start + end) >> 1;
		//edgecase
		if (centroid.max()[axis] == centroid.min()[axis]) {
			for (int i = start; i < end; ++i)
				orderedPrims.emplace_back(prims[i]);
			node->initLeaf(prevOrdered, numPrims, bounds);
			return node;
		}
		else {
			//make a leaf if its less than teh max objects
			if (numPrims <= MAX_BVH_OBJECTS) {
				mid = (start + end) >> 1;
				std::nth_element(&prims[start], &prims[mid], &prims[end - 1] + 1, [axis](Face a, Face b) {
					Bounds ab = a.getBounds();
					Bounds bb = b.getBounds();
					return ab.center[axis] < bb.center[axis];
				});
			}
			else {
				int bestAxis = 0;
				int bestMinCost = numPrims;
				int bestMinCostSplitBucket = INT_MAX;
				constexpr int numBuckets = 16;

				for (int ba = 0; ba < 3; ++ba) {

					//initialize the buckets
					BVHBucket buckets[numBuckets];
					for (int i = start; i < end; ++i) {
						Bounds fb = prims[i].getBounds();
						BVHBounds tempBounds = BVHBounds(fb.center, fb.extents);
						int b = numBuckets * centroid.Offset(fb.center, ba);
						if (b == numBuckets) b--;
						buckets[b].count++;
						buckets[b].bounds = buckets[b].bounds.combine(tempBounds);
					}

					constexpr int nb = numBuckets - 1;
					float cost[nb];
					for (int i = 0; i < nb; ++i) {
						BVHBounds b0, b1;
						int c0 = 0, c1 = 0;

						for (int j = 0; j <= i; ++j) {
							b0 = b0.combine(buckets[j].bounds);
							c0 += buckets[j].count;
						}
						for (int j = i + 1; j < numBuckets; ++j) {
							b1 = b1.combine(buckets[j].bounds);
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
					if (minCost < bestMinCost) {
						bestMinCost = minCost;
						bestMinCostSplitBucket = minCostSplitBucket;
						bestAxis = ba;
					}
					axis = bestAxis;
				}

				if (numPrims > MAX_BVH_OBJECTS || bestMinCost < numPrims) {
					Face* midPtr = std::partition(&prims[start], &prims[end - 1] + 1, [bestAxis, centroid,  bestMinCostSplitBucket, numBuckets](Face& a) {
						int b = (numBuckets)* centroid.Offset(a.getBounds().center, bestAxis);
						if (b == numBuckets) b = numBuckets - 1;
						return b <= bestMinCostSplitBucket;

					});
					mid = midPtr - &prims[0];
					//if (mid != start && mid != end)
					//	break;
					if(mid == start || mid == end){
						for (int i = start; i < end; ++i)
							orderedPrims.emplace_back(prims[i]);
						node->initLeaf(prevOrdered, numPrims, bounds);
						return node;
					}

				}
				else { //create leaf
					for (int i = start; i < end; ++i)
						orderedPrims.emplace_back(prims[i]);
					node->initLeaf(prevOrdered, numPrims, bounds);
					return node;
				}
			} 
			node->initInterior(axis, recursiveBuild(start, mid, totalNodes, orderedPrims, prims), recursiveBuild(mid, end, totalNodes, orderedPrims, prims));
		}


	}
	return node;
	//return std::unique_ptr<BVHNode>();
}

int flattenBVH(std::shared_ptr<BVHNode> node, int * offset, std::vector<flatBVHNode>& bvh)
{
	//first pusch back a node
	flatBVHNode* bvhNode = &bvh[*offset];
	bvhNode->upper = node->bounds.center + node->bounds.extents;
	bvhNode->lower = node->bounds.center - node->bounds.extents;
	//bvhNode.splitAxis = node->splitAxis;

	//increment the offset
	int myOffset = (*offset)++;

	//check if leaf
	if (node->nPrims > 0) {
		bvhNode->numChildren = node->nPrims;
		//bvhNode->numChildren |= (node->splitAxis << 29);
		//bvh[index].numChildren |= (node->splitAxis << 29);
		bvhNode->offset = node->firstPrimOffset;
	} //else make new node
	else {
		flattenBVH(node->children[0], offset, bvh);
		bvhNode->offset = flattenBVH(node->children[1], offset, bvh);
		bvhNode->numChildren = 0;
		//bvhNode->numChildren |= (node->splitAxis << 29);
		//bvh[index].numChildren |= (node->splitAxis << 29);

	}
	return myOffset;
}

void BuildBVH(Mesh& mesh) {
	std::vector<Face> orderedFaces;
	orderedFaces.reserve(mesh.faces.size());
	int tn = 0;
	std::shared_ptr<BVHNode> root = recursiveBuild(0, mesh.faces.size(), &tn, orderedFaces, mesh.faces);
	mesh.faces = std::move(orderedFaces);
	mesh.bvh.resize(tn);
	tn = 0;
	flattenBVH(root, &tn, mesh.bvh);
}

void Build(PrincipiaModel& model) {
	for (auto& m : model.meshes) {
		for (auto& f : m.faces) {
			f.vertices = &m.vertices;
		}
		BuildBVH(m);
	}
}