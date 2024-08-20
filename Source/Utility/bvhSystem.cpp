
#include "../pch.h"
#include "bvhSystem.h"
#include "timer.hpp"
#include <assert.h>
#include "componentIncludes.h"


namespace Principia {

#pragma region helper lambdas

	auto bounds_function = [](const RTCBoundsFunctionArguments* args) {

		PrimitiveComponent* prims = (PrimitiveComponent*)args->geometryUserPtr;
		PrimitiveComponent& prim = prims[args->primID];

		glm::vec3 center = glm::vec3(prim.world[3]);
		glm::vec3 lower = glm::vec3(center - prim.extents);
		glm::vec3 upper = glm::vec3(center + prim.extents);

		args->bounds_o->lower_x = lower.x;
		args->bounds_o->lower_y = lower.y;
		args->bounds_o->lower_z = lower.z;
		args->bounds_o->upper_x = upper.x;
		args->bounds_o->upper_y = upper.y;
		args->bounds_o->upper_z = upper.z;
		};

	auto splitPrimitive = [](const RTCBuildPrimitive* prim, unsigned int dim, float pos, RTCBounds* lprim, RTCBounds* rprim, void* userPtr)
		{
			assert(dim < 3);
			assert(prim->geomID == 0);
			*(BvhBounds*)lprim = *(BvhBounds*)prim;
			*(BvhBounds*)rprim = *(BvhBounds*)prim;
			(&lprim->upper_x)[dim] = pos;
			(&rprim->lower_x)[dim] = pos;
		};
	static int Num_Nodes = 0;
	auto create_inner = [](RTCThreadLocalAllocator alloc, unsigned int numChildren, void* userPtr)
		{
			assert(numChildren == 2);
			void* ptr = rtcThreadLocalAlloc(alloc, sizeof(InnerBvhNode), 16);
			Num_Nodes++;
			return (void*) new (ptr) InnerBvhNode;
		};
	auto setChildren = [](void* BvhNodePtr, void** childPtr, unsigned int numChildren, void* userPtr)
		{
			assert(numChildren == 2);
			for (size_t i = 0; i < 2; i++)
				((InnerBvhNode*)BvhNodePtr)->children[i] = (BvhNode*)childPtr[i];
		};
	auto setBounds = [](void* BvhNodePtr, const RTCBounds** bounds, unsigned int numChildren, void* userPtr)
		{
			assert(numChildren == 2);
			for (size_t i = 0; i < 2; i++)
				((InnerBvhNode*)BvhNodePtr)->bounds[i] = *(const BvhBounds*)bounds[i];
		};
	const auto MAX_LEAF_SIZE = 1;
	const auto MIN_LEAF_SIZE = 1;
	auto create_leaf = [](RTCThreadLocalAllocator alloc, const RTCBuildPrimitive* prims, size_t numPrims, void* userPtr)
		{
			assert(numPrims >= MIN_LEAF_SIZE && numPrims <= MAX_LEAF_SIZE);
			void* ptr = rtcThreadLocalAlloc(alloc, sizeof(LeafBvhNode), 16);
			//std::cout << "Leaf " << prims->primID << ": " << numPrims << std::endl;
			Num_Nodes++;
			return (void*) new (ptr) LeafBvhNode(prims->primID, *(BvhBounds*)prims);
		};
#pragma endregion

	BvhSystem::BvhSystem()
	{
		addComponentType<NodeComponent>();
		addComponentType<TransformComponent>();
		addComponentType<PrimitiveComponent>();

		device = rtcNewDevice(nullptr);
		if (!device) {
			std::cerr << "Error creating Embree device" << std::endl;
			return;
		}
		bvh = rtcNewBVH(device);
		rtcSetDeviceMemoryMonitorFunction(device, [](void* userPtr, ssize_t bytes, bool post) {return true; }, nullptr);
	}

	BvhSystem::~BvhSystem()
	{
		rtcReleaseBVH(bvh);
		rtcReleaseDevice(device);
	}

	void BvhSystem::initialize()
	{
		nodeMapper.init(*world);
		transMapper.init(*world);
		primMapper.init(*world);

		//splitMethod = SplitMethod::EqualsCounts;
	}

	void BvhSystem::build()
	{
		Num_Nodes = 0;
		build_prims.clear();
		build_prims.reserve(prims.size());

		unsigned int i = 0;
		for (auto& prim : prim_comps) {
			auto lower = prim->center() - prim->aabbExtents;
			auto upper = prim->center() + prim->aabbExtents;
			RTCBuildPrimitive build_prim = {
				.lower_x = lower.x,
				.lower_y = lower.y,
				.lower_z = lower.z,
				.geomID = 0,// static_castunsigned int>(prim->id),
				.upper_x = upper.x,
				.upper_y = upper.y,
				.upper_z = upper.z,
				.primID = i
			};

			build_prims.emplace_back(build_prim);
			i++;
		}

		RTCBuildArguments arguments = rtcDefaultBuildArguments();
		arguments.buildQuality = RTC_BUILD_QUALITY_MEDIUM,
		arguments.buildFlags = RTC_BUILD_FLAG_DYNAMIC,
		arguments.maxBranchingFactor = 2,
		arguments.maxDepth = 1024,
		arguments.sahBlockSize = 1,
		arguments.minLeafSize = MIN_LEAF_SIZE,
		arguments.maxLeafSize = MAX_LEAF_SIZE,
		arguments.traversalCost = 1.f,
		arguments.intersectionCost = 1.f,
		arguments.bvh = bvh,
		arguments.primitives = build_prims.data(),
		arguments.primitiveCount = build_prims.size(),
		arguments.primitiveArrayCapacity = build_prims.capacity(),
		arguments.createNode = create_inner,
		arguments.setNodeChildren = setChildren,
		arguments.setNodeBounds = setBounds,
		arguments.createLeaf = create_leaf,
		arguments.splitPrimitive = splitPrimitive,
		//.buildProgress = buildProgress,
		arguments.userPtr = nullptr;

		//bvh_sys->print();
		root = (BvhNode*)rtcBuildBVH(&arguments);
		num_nodes = Num_Nodes;
	}


	void BvhSystem::processEntity(artemis::Entity& e)
	{

	}

	void BvhSystem::added(artemis::Entity& e)
	{
		rebuild = true;
		prims.push_back(&e);
		prim_comps.push_back((PrimitiveComponent*)e.getComponent<PrimitiveComponent>());
		//primMapper.get(e)->bvhIndex = prims.size();
	}

	void BvhSystem::removed(artemis::Entity& e)
	{
		int i = 0;
		for (auto p : prims) {
			if (p == &e)
				break;
			i++;
		}
		prims.erase(prims.begin() + i);
		prim_comps.erase(prim_comps.begin() + i);
	}
}