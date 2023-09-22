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
		artemis::ComponentMapper<NodeComponent> nodeMapper;
		artemis::ComponentMapper<TransformComponent> transMapper;
		artemis::ComponentMapper<PrimitiveComponent> primMapper;
	public:
		BvhSystem();
		~BvhSystem();

		void initialize();
		void build();
		void build_madman();
		void update(artemis::Entity& e);
		void processEntity(artemis::Entity &e);
		void begin();
		void end();
		void added(artemis::Entity &e);
		void removed(artemis::Entity &e);

		inline void setCamera(artemis::Entity* e) {
			cc = (CameraComponent*)e->getComponent<CameraComponent>();
		}
		int cull_count = 0;


		std::vector<artemis::Entity*> prims;
		std::vector<artemis::Entity*> culled_prims;
		std::vector<PrimitiveComponent*> prim_comps;
		std::vector<PrimitiveComponent*> culled_prim_comps;
		SplitMethod splitMethod = SplitMethod::SAH;
		BVHNode* root;
		size_t arena_ptr = 0;
		int totalNodes;
		std::array<int, 4> totalNodesMulti;
		std::array<std::shared_ptr<BVHNode>,4> roots;
		//std::array<std::future<BVHNode*>, 4> future_roots;
		bool rebuild = true;

		Bvh bvh_;
		std::vector<BvhPrim> custom_prims;

		CameraComponent* cc;
		glm::vec4 cam_pos;
		float max_dist = 8;

		struct BvhStepInfo {
			BVHNode* node;
			int mid;
			int axis;
			BvhStepInfo(BVHNode* n, int m, int a) : node(n), mid(m), axis(a) {};
		};

	private:
		void build(TreeType tt, std::vector<artemis::Entity*> &ops);
		void buildCulled(TreeType tt, std::vector<artemis::Entity*>& ops);
		void buildMultiThreaded2(TreeType tt, std::vector<artemis::Entity*>& ops);
		void buildMultiThreadedCulled(TreeType tt, std::vector<artemis::Entity*>& ops);
		void buildMultiThreaded8(TreeType tt, std::vector<artemis::Entity*>& ops);
		BvhStepInfo buildStep(int start, int end, int* totalNodes);
		BvhStepInfo buildStepCulled(int start, int end, int* totalNodes);
		BVHNode* recursiveBuild(int start, int end, int* totalNodes, std::vector<artemis::Entity*> &orderedPrims);
		BVHNode* recursiveBuildCulled(int start, int end, int* totalNodes, std::vector<artemis::Entity*>& orderedPrims);
		BVHNode* hlbvhBuild(int start, int end, int* totalNodes, std::vector<artemis::Entity*>& orderedPrims);
		BVHBounds computeBounds(int s, int e);
		BVHBounds computeCulledBounds(int s, int e);
		BVHBounds computeCentroidBounds(int s, int e);
		inline int chooseAxis(const glm::vec3& center);
		BVHBounds computeBoundsMulti(int s, int e);
		BVHBounds computeCentroidBoundsMulti(int s, int e);
		void FrustrumCull(artemis::Entity& e);

	};

}