#pragma once
#include "bvhComponent.hpp"
//#include "../Utility/transformComponent.hpp"
//#include "../Rendering/Components/renderComponents.hpp"
#include "componentIncludes.h"

namespace Principia {
	static const int MAX_BVH_OBJECTS = 4;

	class BvhSystem : public artemis::EntityProcessingSystem
	{
	private:
		//artemis::ComponentMapper<NodeComponent> nodeMapper;
		artemis::ComponentMapper<TransformComponent> transMapper;
		artemis::ComponentMapper<PrimitiveComponent> primMapper;
	public:
		BvhSystem();
		~BvhSystem();

		void initialize();
		void build();
		void processEntity(artemis::Entity &e);
		void begin();
		void end();
		void added(artemis::Entity &e);
		void removed(artemis::Entity &e);


		std::vector<artemis::Entity*> prims;
		SplitMethod splitMethod = SplitMethod::SAH;
		BVHNode* root;
		int totalNodes;
		std::array<int, 4> totalNodesMulti;
		std::array<std::shared_ptr<BVHNode>,4> roots;
		bool rebuild = true;

	private:
		void build(TreeType tt, std::vector<artemis::Entity*> &ops);
		void buildMultiThreaded(TreeType tt, std::vector<artemis::Entity*>& ops);
		BVHNode* recursiveBuild(int start, int end, int* totalNodes, std::vector<artemis::Entity*> &orderedPrims);
		BVHNode* hlbvhBuild(int start, int end, int* totalNodes, std::vector<artemis::Entity*>& orderedPrims);
		BVHBounds computeBounds(int s, int e);
		BVHBounds computeCentroidBounds(int s, int e);
		inline int chooseAxis(const glm::vec3& center);
		BVHBounds computeBoundsMulti(std::array<std::shared_ptr<BVHNode>, 4> nodes);
	};

}