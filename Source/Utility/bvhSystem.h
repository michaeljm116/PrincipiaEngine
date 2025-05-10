#pragma once	
#include "bvhComponent.hpp"
#include <embree4/rtcore.h>

namespace Principia {

	struct NodeComponent;
	struct TransformComponent;
	struct PrimitiveComponent;

	class BvhSystem : public artemis::EntityProcessingSystem
	{
	public:
		BvhSystem();
		~BvhSystem();

		void initialize();
		void build();
		void processEntity(artemis::Entity &e);
		void added(artemis::Entity &e);
		void removed(artemis::Entity &e);

		inline BvhNode* get_root() const { return root; }
		inline std::vector<RTCBuildPrimitive> get_ordered_prims() const { return build_prims; }
		inline int get_num_nodes() { return num_nodes; }
		inline std::vector<artemis::Entity*> get_original_prims() { return prims; }

		bool rebuild = false;
	private:
		RTCDevice device = {};
		RTCBVH bvh = {};
		BvhNode* root = nullptr;
		std::vector<artemis::Entity*> prims;
		std::vector<PrimitiveComponent*> prim_comps;
		std::vector<RTCBuildPrimitive> build_prims = {};
		int num_nodes = 0;

		artemis::ComponentMapper<NodeComponent> nodeMapper = {};
		artemis::ComponentMapper<TransformComponent> transMapper = {};
		artemis::ComponentMapper<PrimitiveComponent> primMapper = {};
	};

}