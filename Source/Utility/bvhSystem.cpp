
#include "../pch.h"
#include "bvhSystem.h"
#include "timer.hpp"
BvhSystem::BvhSystem()
{
	addComponentType<TransformComponent>();
	addComponentType<PrimitiveComponent>();
}

BvhSystem::~BvhSystem()
{
}

void BvhSystem::initialize()
{
	transMapper.init(*world);
	primMapper.init(*world);
}

void BvhSystem::build()
{

	if (rebuild) {
		std::vector<artemis::Entity*> orderedPrims;

		//reserve data
		size_t count = getEntityCount();
		prims.reserve(count);
		orderedPrims.reserve(count);

		//load up entities
		//process();

		//Build the bvh
		//Principia::NamedTimer bvhTime("BVH Build");

		build(TreeType::Recursive, orderedPrims);
		rebuild = false;
	}

}

void BvhSystem::processEntity(artemis::Entity & e)
{
	//prims.emplace_back(&e);

}

void BvhSystem::begin()
{

}

void BvhSystem::end()
{
	//if (rebuild == true)
	//	build();
	//rebuild = false;
}

void BvhSystem::added(artemis::Entity & e)
{
	rebuild = true;
	prims.push_back(&e);
}

void BvhSystem::removed(artemis::Entity & e)
{
}

void BvhSystem::build(TreeType tt, std::vector<artemis::Entity*> &ops)
{
	totalNodes = 0;

	root = recursiveBuild(0, prims.size(), &totalNodes, ops);

	prims = std::move(ops);
	//prims = ops;
}
//(Recall that the number of nodes in a BVH is bounded by twice the number of leaf nodes, which in turn is bounded by the number of primitives)
std::shared_ptr<BVHNode> BvhSystem::recursiveBuild(int start, int end, int * totalNodes, std::vector<artemis::Entity*> &orderedPrims)
{
	*totalNodes += 1;
	std::shared_ptr<BVHNode> node(new BVHNode);// std::make_shared<BVHNode>();
	BVHBounds bounds = computeBounds(start, end);

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
		BVHBounds centroid = computeCentroidBounds(start, end);
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
			artemis::ComponentMapper<PrimitiveComponent>* ptm = &primMapper;
			switch (splitMethod) {
			case SplitMethod::Middle: {
				artemis::Entity **midPtr = std::partition(&prims[start], &prims[end - 1] + 1, [axis, centroid, ptm](artemis::Entity * a) {
					return ptm->get(*a)->center[axis] < centroid.center[axis];
				});
				mid = midPtr - &prims[0];
			}
			case SplitMethod::EqualsCounts: {
				std::nth_element(&prims[start], &prims[mid], &prims[end - 1] + 1, [axis, ptm](artemis::Entity* a, artemis::Entity* b) {
					return ptm->get(*a)->center[axis] < ptm->get(*b)->center[axis];
				});
			}
			case SplitMethod::SAH: {
				if (numPrims <= MAX_BVH_OBJECTS) {
					mid = (start + end) >> 1;
					std::nth_element(&prims[start], &prims[mid], &prims[end - 1] + 1, [axis, ptm](artemis::Entity* a, artemis::Entity* b) {
						return ptm->get(*a)->center[axis] < ptm->get(*b)->center[axis];
					});
				}
				else {
					//initialize the buckets
					constexpr int numBuckets = 12;
					BVHBucket buckets[numBuckets];
					for (int i = start; i < end; ++i) {
						PrimitiveComponent* pc = ptm->get(*prims[i]);
						BVHBounds tempBounds = BVHBounds(pc->center, pc->extents);
						int b = numBuckets * centroid.Offset(pc->center, axis);
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
					float leafCost = numPrims;
					if (numPrims > MAX_BVH_OBJECTS || minCost < leafCost) {
						artemis::Entity **midPtr = std::partition(&prims[start], &prims[end - 1] + 1, [axis, centroid, ptm, minCostSplitBucket, numBuckets](artemis::Entity * a) {
							int b = (numBuckets)* centroid.Offset(ptm->get(*a)->center, axis);
							if (b == numBuckets) b = numBuckets - 1;
							return b <= minCostSplitBucket;

						});
						mid = midPtr - &prims[0];
						if (mid != start && mid != end)
							break;
						else {
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
			}
			default:
				break;
			}

			node->initInterior(axis, recursiveBuild(start, mid, totalNodes, orderedPrims), recursiveBuild(mid, end, totalNodes, orderedPrims));
		}
	}
	return node;
	//return std::unique_ptr<BVHNode>();
}

BVHBounds BvhSystem::computeBounds(int s, int e)
{
	//make an aabb of the entire scene basically
	//find the minimum x and maximum x and bounds = max-min/2 = center
	glm::vec3 min(FLT_MAX);
	glm::vec3 max(-FLT_MAX);
	for (int i = s; i < e; ++i) {
		//TransformComponent* tc = transMapper.get(*prims[i]);
		PrimitiveComponent* pc = primMapper.get(*prims[i]);
		//AABBComponent* bc = boundsMapper.get(*prims[i]);
		//min = tulip::minV(min, glm::vec3(tc->world[3]) + glm::vec3(tc->global.scale));
		//max = tulip::maxV(max, glm::vec3(tc->world[3]) + glm::vec3(tc->global.scale));
		min = tulip::minV(min, glm::vec3(pc->center) - glm::vec3(pc->extents));
		max = tulip::maxV(max, glm::vec3(pc->center) + glm::vec3(pc->extents));
	}
	glm::vec3 c = (max + min) * 0.5f;
	glm::vec3 ex = max - c;

	return BVHBounds(c, ex);
}

BVHBounds BvhSystem::computeCentroidBounds(int s, int e)
{
	//make an aabb of the entire scene basically
	//find the minimum x and maximum x and bounds = max-min/2 = center
	glm::vec3 min(FLT_MAX);
	glm::vec3 max(-FLT_MAX);
	for (int i = s; i < e; ++i) {
		//TransformComponent* tc = transMapper.get(*prims[i]);
		//AABBComponent* bc = boundsMapper.get(*prims[i]);
		PrimitiveComponent* pc = primMapper.get(*prims[i]);
		//min = tulip::minV(min, glm::vec3(tc->world[3]) + glm::vec3(tc->global.scale));
		//max = tulip::maxV(max, glm::vec3(tc->world[3]) + glm::vec3(tc->global.scale));
		min = tulip::minV(min, pc->center - pc->extents);
		max = tulip::maxV(max, pc->center + pc->extents);
	}
	glm::vec3 c = (max + min) * 0.5f;
	glm::vec3 ex = max - c;

	return BVHBounds(c, ex);
}

int BvhSystem::chooseAxis(const glm::vec3& c) {
	if ((c.x > c.y) && (c.x > c.z)) return 0;
	else if ((c.y > c.x) && (c.y > c.z)) return 1;
	else return 2;
}

