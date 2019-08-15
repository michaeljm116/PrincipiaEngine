#pragma once
#include "nodeComponent.hpp"
#include "../Game/transformComponent.hpp"
#include "bvh.hpp"

class BvhSystem : public artemis::EntityProcessingSystem
{
private:
	//artemis::ComponentMapper<NodeComponent> nodeMapper;
	artemis::ComponentMapper<TransformComponent> transMapper;
	std::vector<artemis::Entity*> prims;
	std::vector<artemis::Entity*> orderedPrims;
	SplitMethod splitMethod = SplitMethod::EqualsCounts;
public:
	BvhSystem();
	~BvhSystem();

	void initialize();
	void processEntity(artemis::Entity &e);
	void begin();
	void end();
	void added(artemis::Entity &e);
	void removed(artemis::Entity &e);

private:
	void build(SplitMethod sm, TreeType tt);
	std::unique_ptr<BVHNode> recursiveBuild(int start, int end, int* totalNodes);
	BVHBounds computeBounds(int s, int e);
	glm::vec3 computeCenter(int s, int e);
};

