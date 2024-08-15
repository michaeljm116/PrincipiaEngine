
#include "../pch.h"
#include "bvhSystem.h"
#include "timer.h"
#include <future>


namespace Principia {
	static const int MAX_BVH_OBJECTS = 2;
	BvhSystem::BvhSystem()
	{
		addComponentType<NodeComponent>();
		addComponentType<TransformComponent>();
		addComponentType<PrimitiveComponent>();

		root = new BVHNode[0x20000];
	}

	BvhSystem::~BvhSystem()
	{
		//if (root) {
		//	delete[] root;
		//	free(root);
		//	root = nullptr;
		//}
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
		if (!rebuild)
			//delete root;
			arena_ptr = 0;
		//if (rebuild) {
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
		//buildMultiThreaded8(TreeType::Recursive, orderedPrims);
		rebuild = false;
		//}

	}
	void BvhSystem::build_madman()
	{
		//Build the list of Primitives
		custom_prims.clear();
		custom_prims.reserve(prims.size());
		for (auto* e : prims) {
			auto* p = primMapper.get(*e);
			custom_prims.emplace_back(BvhPrim(p));
		}

		//Convert the prims into something the bvh can use
		auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(custom_prims.data(), custom_prims.size());
		auto global_bbox = bvh::compute_bounding_boxes_union(bboxes.get(), custom_prims.size());
		//bvh::LinearBvhBuilder<Bvh, Morton> builder(bvh_);
		//bvh::SweepSahBuilder<Bvh> builder(bvh_);
		bvh::LocallyOrderedClusteringBuilder<Bvh, Morton> builder(bvh_);
		builder.build(global_bbox, bboxes.get(), centers.get(), custom_prims.size());

		//convert the bvh to something the engine can use
		// - List of Prims
		// - List of Nodes

	}

	void BvhSystem::update(artemis::Entity& e)
	{
	}

	void BvhSystem::processEntity(artemis::Entity & e)
	{
	
	}

	void BvhSystem::begin()
	{

	}

	void BvhSystem::end()
	{
		std::vector<artemis::Entity*> orderedPrims;
		build(TreeType::Recursive, orderedPrims);
		rebuild = false;
		//std::cout << "\nEntities: " << countz0  << " ";
	}

	void BvhSystem::added(artemis::Entity & e)
	{
		rebuild = true;
		prims.push_back(&e);
		prim_comps.push_back((PrimitiveComponent*)e.getComponent<PrimitiveComponent>());
		//primMapper.get(e)->bvhIndex = prims.size();
	}

	void BvhSystem::removed(artemis::Entity & e)
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

	void BvhSystem::build(TreeType tt, std::vector<artemis::Entity*> &ops)
	{
		totalNodes = 0;

		root = recursiveBuild(0, prims.size(), &totalNodes, ops);

		prims = std::move(ops);
		//prims = ops;
	}

	void BvhSystem::buildMultiThreaded2(TreeType tt, std::vector<artemis::Entity*>& ops)
	{
		totalNodes = 0;
		auto step1 = buildStep(0, prims.size(), &totalNodes);


		std::vector<artemis::Entity*> ordered_prims0;
		std::vector<artemis::Entity*> ordered_prims1;
		std::array<std::future<BVHNode*>, 2> future_nodes;
		std::array<BVHNode*, 2 > nodes;
		std::array<int, 2> total_nodes = { 0,0};
		std::array<int, 3> ranges = { 0, step1.mid, prims.size() };


//#pragma omp parallel for
//		for (int i = 0; i < 2; ++i) {
//			//nodes[i] = future_nodes[i].get();
//			nodes[i] = recursiveBuild(ranges[i], ranges[i + 1], &total_nodes[i], ordered_prims[i]);
//		}

		//for (int i = 0; i < 2; ++i) {
		//	future_nodes[i] = std::async(std::launch::async, &BvhSystem::recursiveBuild, this, ranges[i], ranges[i+1], &total_nodes[i], std::ref(ordered_prims[i]));
		//}
		future_nodes[0] = std::async(std::launch::async, &BvhSystem::recursiveBuild, this, ranges[0], ranges[1], &total_nodes[0], std::ref(ordered_prims0));
		future_nodes[1] = std::async(std::launch::async, &BvhSystem::recursiveBuild, this, ranges[1], ranges[2], &total_nodes[1], std::ref(ordered_prims1));


		for (int i = 0; i < 2; ++i) {
			nodes[i] = future_nodes[i].get();
		}
		for (int i = 0; i < 2; ++i) {
			totalNodes += total_nodes[i];
			//ops.insert(ops.begin(), ordered_prims[i].begin(), ordered_prims[i].end());
		}
		ops.insert(ops.begin(), ordered_prims0.begin(), ordered_prims0.end());
		ops.insert(ops.end(), ordered_prims1.begin(), ordered_prims1.end());

		prims = std::move(ops);

		step1.node->initInterior(step1.axis, nodes[0], nodes[1]);

		root = step1.node;

		std::cout << "\nNumCores: " << std::thread::hardware_concurrency() << std::endl;
	}

	void BvhSystem::buildMultiThreaded8(TreeType tt, std::vector<artemis::Entity*>& ops)
	{
		totalNodes = 0;
		std::array<std::future<BvhStepInfo>, 7> build_step_futures;
		auto step1 = buildStep(0, prims.size(), &totalNodes);

		//int step2LNodes = 0;
		//std::vector<artemis::Enti
		//auto step2L_future = std::async(std::launch::async, &BvhSystem::buildStep, this, 0, step1.mid, &step2LNodes);

		auto step2L = buildStep(0, step1.mid, &totalNodes);
		auto step2R = buildStep(step1.mid, prims.size(), &totalNodes);

		auto step3LL = buildStep(0, step2L.mid, &totalNodes);
		auto step3LR = buildStep(step2L.mid, step1.mid, &totalNodes);
		auto step3RL = buildStep(step1.mid, step2R.mid, &totalNodes);
		auto step3RR = buildStep(step2R.mid, prims.size(), &totalNodes);


		std::array<std::vector<artemis::Entity*>, 8> ordered_prims;
		std::array<std::future<BVHNode*>, 8> future_nodes;
		std::array<BVHNode*, 8 > nodes;
		std::array<int, 8> total_nodes = { 0,0,0,0,0,0,0,0 };
		std::array<int, 9> ranges = { 0, step3LL.mid, step2L.mid, step3LR.mid, step1.mid, step3RL.mid, step2R.mid, step3RR.mid, prims.size() };

		
		//future_nodes[0] = std::async(std::launch::async, &BvhSystem::recursiveBuild, this, 0, step3LL.mid, &total_nodes[0], std::ref(ordered_prims[0]));
		//future_nodes[1] = std::async(std::launch::async, &BvhSystem::recursiveBuild, this, step3LL.mid, step2L.mid, &total_nodes[1], std::ref(ordered_prims[1]));
		//future_nodes[2] = std::async(std::launch::async, &BvhSystem::recursiveBuild, this, step2L.mid, step3LR.mid, &total_nodes[2], std::ref(ordered_prims[2]));
		//future_nodes[3] = std::async(std::launch::async, &BvhSystem::recursiveBuild, this, step3LR.mid, step1.mid, &total_nodes[3], std::ref(ordered_prims[3]));
		//future_nodes[4] = std::async(std::launch::async, &BvhSystem::recursiveBuild, this, step1.mid, step3RL.mid, &total_nodes[4], std::ref(ordered_prims[4]));
		//future_nodes[5] = std::async(std::launch::async, &BvhSystem::recursiveBuild, this, step3RL.mid, step2R.mid, &total_nodes[5], std::ref(ordered_prims[5]));
		//future_nodes[6] = std::async(std::launch::async, &BvhSystem::recursiveBuild, this, step2R.mid, step3RR.mid, &total_nodes[6], std::ref(ordered_prims[6]));
		//future_nodes[7] = std::async(std::launch::async, &BvhSystem::recursiveBuild, this, step3RR.mid, prims.size(), &total_nodes[7], std::ref(ordered_prims[7]));

		//for (int i = 0; i < 8; ++i) {
		//	future_nodes[i] = std::async(std::launch::async, &BvhSystem::recursiveBuild, this, ranges[i], ranges[i+1], &total_nodes[i], std::ref(ordered_prims[i]));
		//}

		
#pragma omp parallel for
		for (int i = 0; i < 8; ++i) {
			//nodes[i] = future_nodes[i].get();
			nodes[i] = recursiveBuild(ranges[i], ranges[i + 1], &total_nodes[i], ordered_prims[i]);
		}
		

		//for (int i = 0; i < 8; ++i) {
		//	nodes[i] = future_nodes[i].get();
		//}
		for (int i = 0; i < 8; ++i) {
			totalNodes += total_nodes[i];
			ops.insert(ops.end(), ordered_prims[i].begin(), ordered_prims[i].end());
		}
		prims = std::move(ops);

		step3RR.node->initInterior(step3RR.axis, nodes[6], nodes[7]);
		step3RL.node->initInterior(step3RL.axis, nodes[4], nodes[5]);
		step3LR.node->initInterior(step3LR.axis, nodes[2], nodes[3]);
		step3LL.node->initInterior(step3LL.axis, nodes[0], nodes[1]);

		step2L.node->initInterior(step2L.axis, step3LL.node, step3LR.node);
		step2R.node->initInterior(step2R.axis, step3RL.node, step3RR.node);

		step1.node->initInterior(step1.axis,step2L.node, step2R.node);

		root = step1.node;

		std::cout << "\nNumCores: " << std::thread::hardware_concurrency() << std::endl;
		/*
		future_roots[0] = std::async(std::launch::async, &BvhSystem::recursiveBuild, this, 0, step1.mid, &totalNodes, ops);
		auto b = std::async(std::launch::async, &BvhSystem::recursiveBuild, this, step1.mid, prims.size(), &totalNodes, ops);

		auto tester = std::async(std::launch::async, &BvhSystem::asynctester, this, 0, step1.mid, &totalNodes);

		std::thread at(recursiveBuild(0, step1.mid, &totalNodes, ops));
		std::thread bt(recursiveBuild(step1.mid, prims.size(), &totalNodes, ops));

		at.join();
		bt.join();

		
		totalNodesMulti = {0, 0, 0, 0};
		auto size = ops.size();
		int quartSize = size / 4;
		int remainder = size % 4;
		std::array<std::vector<artemis::Entity*>, 4> ordered_prims;
		for (int i = 0; i < 4; ++i) {
			if (i != 3)
				ordered_prims[i].reserve(quartSize);
			else
				ordered_prims[i].reserve(quartSize + remainder);
		}*/

		//std::thread first(recursiveBuild(0, quartSize, &totalNodesMulti[0], ordered_prims[0]));
		//std::thread second(recursiveBuild(quartSize, quartSize * 2, &totalNodesMulti[1], ordered_prims[1]));
		//std::thread third(recursiveBuild(quartSize * 2, quartSize * 3, &totalNodesMulti[2], ordered_prims[2]));
		//std::thread fourth(recursiveBuild(quartSize * 3, quartSize * 4 + remainder, &totalNodesMulti[3], ordered_prims[3]));
		//std::thread first(&recursiveBuild, 0, quartSize, &totalNodesMulti[0], ordered_prims[0]);
		//std::array<std::future<BVHNode*>, 4> futures;
		//for (int i = 0; i < 4; ++i) {
		//	if (i < 3)
		//		futures[i] = std::async(std::launch::async, &BvhSystem::recursiveBuild, this, i * quartSize, (i + 1) * quartSize, &totalNodesMulti[0], ordered_prims[0]);
		//	else
		//		futures[i] = std::async(std::launch::async, &BvhSystem::recursiveBuild, this, i * quartSize, (i + 1) * quartSize + remainder, &totalNodesMulti[0], ordered_prims[0]);
		//}

		//for (int i = 0; i < 4; ++i) {
		//	roots[i] = futures[i].get();
		//}
		//for (int i = 0; i < 4; ++i) {
		//	ops.emplace(ordered_prims[i].begin(), ordered_prims[i].end());
		//}
		//for (int i = 0; i < 4; ++i) {
		//	root->children[i] = std::move(roots[i]);
		//}

		//root->bounds.combine(computeBoundsMulti(roots));

		//prims = std::move(ops);
	}

	BvhSystem::BvhStepInfo BvhSystem::buildStep(int start, int end, int* totalNodes)
	{
		*totalNodes += 1;
		BVHNode* node(new BVHNode);// std::make_shared<BVHNode>();
		BVHBounds bounds = computeBoundsMulti(start, end);

		//Check if leaf
		int numPrims = end - start;
		//int prevOrdered = orderedPrims.size();
		
		//Not a leaf, create a new node
		
		BVHBounds centroid = computeCentroidBounds(start, end);
		int axis = chooseAxis(centroid.center);
		int mid = (start + end) >> 1;

		artemis::ComponentMapper<PrimitiveComponent>* ptm = &primMapper;		
		//Perform the Surface Area Heuristic
		constexpr int numBuckets = 12;
		BVHBucket buckets[numBuckets];
		for (int i = start; i < end; ++i) {
			PrimitiveComponent* pc = ptm->get(*prims[i]);
			BVHBounds tempBounds = BVHBounds(pc->center(), pc->aabbExtents);
			int b = numBuckets * centroid.Offset(pc->center(), axis);
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
		//node->initInterior(axis, recursiveBuild(start, mid, totalNodes, orderedPrims), recursiveBuild(mid, end, totalNodes, orderedPrims));
		return BvhStepInfo(node, mid, axis);		
	}

	//(Recall that the number of nodes in a BVH is bounded by twice the number of leaf nodes, which in turn is bounded by the number of primitives)
	BVHNode* BvhSystem::recursiveBuild(int start, int end, int * totalNodes, std::vector<artemis::Entity*> &orderedPrims)
	{
		*totalNodes += 1;
		BVHNode* node = new(root, arena_ptr) BVHNode(); // new BVHNode(root, arena_ptr);//(new BVHNode);// std::make_shared<BVHNode>();
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
						return ptm->get(*a)->center()[axis] < centroid.center[axis];
					});
					mid = midPtr - &prims[0];
					break;
				}
				case SplitMethod::EqualsCounts: {
					std::nth_element(&prims[start], &prims[mid], &prims[end - 1] + 1, [axis, ptm](artemis::Entity* a, artemis::Entity* b) {
						return ptm->get(*a)->center()[axis] < ptm->get(*b)->center()[axis];
					});
				}
				case SplitMethod::SAH: {
					if (numPrims <= MAX_BVH_OBJECTS) {
						mid = (start + end) >> 1;
						std::nth_element(&prims[start], &prims[mid], &prims[end - 1] + 1, [axis, ptm](artemis::Entity* a, artemis::Entity* b) {
							return ptm->get(*a)->center()[axis] < ptm->get(*b)->center()[axis];
						});
					}
					else {
						//initialize the buckets
						constexpr int numBuckets = 12;
						BVHBucket buckets[numBuckets];
						for (int i = start; i < end; ++i) {
							PrimitiveComponent* pc = ptm->get(*prims[i]);
							BVHBounds tempBounds = BVHBounds(pc->center(), pc->aabbExtents);
							int b = numBuckets * centroid.Offset(pc->center(), axis);
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
								int b = (numBuckets)* centroid.Offset(ptm->get(*a)->center(), axis);
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
					break;
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

	BVHNode* BvhSystem::hlbvhBuild(int start, int end, int* totalNodes, std::vector<artemis::Entity*>& orderedPrims)
	{
		// Compute bounding box of all primitive centroids
		BVHNode* node(new BVHNode);
		BVHBounds bounds = computeBounds(start, end);

		// Compute Morton indices of primitives
		std::vector<MortonPrimitive> morton_prims(prims.size());
#pragma omp parallel for
		for (int i = 0; i < orderedPrims.size(); ++i) {
			// Initialize _mortonPrims[i]_ for _i_th primitive
			constexpr int morton_bits = 10;
			constexpr int morton_scale = 1 << morton_bits;
			morton_prims[i].primitive_index = i;
			glm::vec3 centroid_offset = bounds.Offset(primMapper.get(*prims[i])->center());
			glm::vec3 offset = centroid_offset * glm::vec3(morton_scale); //TODO: POSSIBLE ERROR
			morton_prims[i].morton_code = EncodeMorton3(offset.x, offset.y, offset.z);
		}
		// Radix sort primitive Morton indices
		RadixSort(&morton_prims);

		// Create LBVH treelets at bottom of BVH
		// Find intervals of primitives for each treelet
		 
		std::vector<LBVHTreelet> treelets_to_build;
		for (size_t start = 0, end = 1; end <= morton_prims.size(); ++end) {
			uint32_t mask = 0b00111111111111000000000000000000;
			if (end == (int)morton_prims.size() || (morton_prims[start].morton_code & mask) !=
				(morton_prims[end].morton_code & mask)) {
				// Add entry to _treeletsToBuild_ for this treelet
				size_t n_primitives = end - start;
				int maxBVHNodes = 2 * n_primitives - 1;
				BVHNode* nodes = nullptr;// TODO ALLOCATE ENOUGH TREELETS TO THE STUFF;
				treelets_to_build.push_back({start, n_primitives, nodes});

				start = end;
			}

		}

		// Create LBVHs for treelets in parallel
		std::atomic<int> ordered_primts_offset(0);
#pragma omp parallel for
		for (int i = 0; i < treelets_to_build.size(); ++i) {
			// Generate _i_th LBVH treelet
			int nodes_created = 0;
			const int first_bit_index = 29 - 12;
			LBVHTreelet& tr = treelets_to_build[i]; //MAKE SURE ALLOCATOR WORKS FIRST
			//tr.build_nodes = ASDF;LKJ
		}
		// Create and return SAH BVH from LBVH treelets
		return nullptr;
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
			min = tulip::minV(min, pc->center() - glm::vec3(pc->aabbExtents));
			max = tulip::maxV(max, pc->center() + glm::vec3(pc->aabbExtents));
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
			min = tulip::minV(min, pc->center() - pc->aabbExtents);
			max = tulip::maxV(max, pc->center() + pc->aabbExtents);
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

	BVHBounds BvhSystem::computeBoundsMulti(int s, int e) 
	{
		glm::vec3 min(FLT_MAX);
		glm::vec3 max(-FLT_MAX);
#pragma omp parallel for
		for (int i = s; i < e; ++i) {
			min = tulip::minV(min, prim_comps[i]->center() - prim_comps[i]->aabbExtents);
			max = tulip::maxV(max, prim_comps[i]->center() + prim_comps[i]->aabbExtents);
		}
		glm::vec3 c = (max + min) * 0.5f;
		glm::vec3 ex = max - c;

		return BVHBounds(c, ex);
	}

	BVHBounds BvhSystem::computeCentroidBoundsMulti(int s, int e) 
	{
		glm::vec3 min(FLT_MAX);
		glm::vec3 max(-FLT_MAX);
#pragma omp parallel for
		for (int i = s; i < e; ++i) {
			min = tulip::minV(min, prim_comps[i]->center() - prim_comps[i]->aabbExtents);
			max = tulip::maxV(max, prim_comps[i]->center() + prim_comps[i]->aabbExtents);
		}
		glm::vec3 c = (max + min) * 0.5f;
		glm::vec3 ex = max - c;

		return BVHBounds(c, ex);
	}
	void BvhSystem::print()
	{
		print(root);
	}
	void BvhSystem::print(BVHNode* n)
	{
		if (!n) return;
		std::cout << "\nCenter X: " << n->bounds.center.x << " Center Y: " << n->bounds.center.y << " Center Z: " << n->bounds.center.z;
		std::cout << "\nExtent X: " << n->bounds.extents.x << " Extent Y: " << n->bounds.extents.y << " Extent Z: " << n->bounds.extents.z;

		for (size_t i = 0; i < 2; ++i) {
			print(n->children[i]);
		}
	}

}