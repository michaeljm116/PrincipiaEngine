#include "bvhSystem.h"

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
		process();

		//Build the bvh
		build(SplitMethod::Middle, TreeType::Recursive, orderedPrims);
		rebuild = false;
	}

}

void BvhSystem::processEntity(artemis::Entity & e)
{
	prims.emplace_back(&e);
}

void BvhSystem::begin()
{
	for (auto p : prims) {

	}
}

void BvhSystem::end()
{
}

void BvhSystem::added(artemis::Entity & e)
{
	rebuild = true;
	//prims.push_back(&e);
}

void BvhSystem::removed(artemis::Entity & e)
{
}

void BvhSystem::build(SplitMethod sm, TreeType tt, std::vector<artemis::Entity*> &ops)
{
	totalNodes = 0;

	root = recursiveBuild(0, prims.size(), &totalNodes, ops);

	prims = std::move(ops);
	//prims = ops;
}

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
		int mid = (start + end) / 2;
		
		//edgecase
		if (centroid.max()[axis] == centroid.min()[axis]) {
			for (int i = start; i < end; ++i)
				orderedPrims.emplace_back(prims[i]);
			node->initLeaf(prevOrdered, numPrims, bounds);
			return node;
		}
		else {
			artemis::ComponentMapper<TransformComponent>* ptm = &transMapper;
			switch (splitMethod) {
			case SplitMethod::Middle: {
				artemis::Entity **midPtr = std::partition(&prims[start], &prims[end - 1] + 1, [axis, centroid, ptm](artemis::Entity * a) {
					return ptm->get(*a)->world[3][axis] < centroid.center[axis];
				});
				mid = midPtr - &prims[0];
			}
			case SplitMethod::EqualsCounts: {
				mid = (start + end) / 2;
				//std::nth_element(&prims[start], &prims[mid], &prims[end - 1] + 1, [axis, ptm](artemis::Entity& a, artemis::Entity& b) {
				//	return ptm->get(a)->world[3][axis] < ptm->get(b)->world[3][axis];
				//});
				//break;
			}
			case SplitMethod::SAH: {}
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
		TransformComponent* tc = transMapper.get(*prims[i]);
		PrimitiveComponent* pc = primMapper.get(*prims[i]);
		//AABBComponent* bc = boundsMapper.get(*prims[i]);
		min = tulip::minV(min, glm::vec3(tc->world[3]) + glm::vec3(tc->global.scale));
		max = tulip::maxV(max, glm::vec3(tc->world[3]) + glm::vec3(tc->global.scale));
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
		TransformComponent* tc = transMapper.get(*prims[i]);
		//AABBComponent* bc = boundsMapper.get(*prims[i]);
		min = tulip::minV(min, glm::vec3(tc->world[3]) + glm::vec3(tc->global.scale));
		max = tulip::maxV(max, glm::vec3(tc->world[3]) + glm::vec3(tc->global.scale));
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

