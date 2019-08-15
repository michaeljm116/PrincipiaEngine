#include "bvhSystem.h"



BvhSystem::BvhSystem()
{
	addComponentType<TransformComponent>();
}

BvhSystem::~BvhSystem()
{
	transMapper.init(*world);
}

void BvhSystem::initialize()
{
}

void BvhSystem::processEntity(artemis::Entity & e)
{
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
	prims.push_back(&e);
}

void BvhSystem::removed(artemis::Entity & e)
{
}

void BvhSystem::build(SplitMethod sm, TreeType tt)
{
	std::unique_ptr<BVHNode> root;
	int totalNodes = 0;

	root = recursiveBuild(0, prims.size(), &totalNodes);

	int a = 4;
}

std::unique_ptr<BVHNode> BvhSystem::recursiveBuild(int start, int end, int * totalNodes)
{
	*totalNodes += 1;
	std::unique_ptr<BVHNode> node = std::make_unique<BVHNode>();
	BVHBounds bounds = computeBounds(start, end);

	//Check if leaf
	int numPrims = end - start;
	if (numPrims < 3) { //create leaf
		int firstPrimOffset = orderedPrims.size();
		for (int i = start; i < end; ++i)
			orderedPrims.push_back(prims[i]);
		node->initLeaf(firstPrimOffset, numPrims, bounds);
	}
	//Not a leaf, create a new node
	else {
		glm::vec3 center = computeCenter(start, end);
		int axis = 0;
		center.x > center.y ? axis = 0 : axis = 1;
		for (int i = 1; i < 3; ++i){
			if (center[i] > center[axis])
				axis = i;
		}

		int mid;

		artemis::ComponentMapper<TransformComponent>* ptm = &transMapper;
		switch (splitMethod) {
			case SplitMethod::Middle: {
				//float pmid = center[axis];
				//artemis::Entity** midPtr = std::partition(&prims[start], &prims[end - 1] + 1, [axis, pmid, ptm](artemis::Entity& e) {
				//	return ptm->get(e)->world[3][axis] < pmid;
				//});
				//mid = midPtr - &prims[0];
				//if (mid != start && mid != end)
				//	break;
			}
			case SplitMethod::EqualsCounts: {
				mid = (start + end) / 2;
				//std::nth_element(&prims[start], &prims[mid], &prims[end - 1] + 1, [axis, ptm](artemis::Entity& a, artemis::Entity& b) {
				//	return ptm->get(a)->world[3][axis] < ptm->get(b)->world[3][axis];
				//});
				//break;
			}
			case SplitMethod::SAH:{}
			default: 
				break;
		}

		node->initInterior(axis, recursiveBuild(start, mid, totalNodes), recursiveBuild(mid, end, totalNodes));
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
		min = tulip::minV(min, glm::vec3(tc->world[3]) + glm::vec3(tc->global.scale));
		max = tulip::maxV(max, glm::vec3(tc->world[3]) + glm::vec3(tc->global.scale));
	}
	glm::vec3 c = (max + min) * 0.5f;
	glm::vec3 ex = max - c;

	return BVHBounds(c, ex);
}

glm::vec3 BvhSystem::computeCenter(int s, int e)
{
	glm::vec3 min(FLT_MAX);
	glm::vec3 max(-FLT_MAX);
	for (int i = s; i < e; ++i) {
		TransformComponent* tc = transMapper.get(*prims[i]);
		min.length();
		min = tulip::minV(min, glm::vec3(tc->world[3]));
		max = tulip::maxV(max, glm::vec3(tc->world[3]));
	}

	return glm::vec3((max - min) * 0.5f);
}
