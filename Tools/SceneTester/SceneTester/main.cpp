//#include "sceneLoader.hpp"
//#include "bvh.hpp"
//../../../Assets/Levels/
#include "sceneLoader.hpp"
#include "xxhash.hpp"

#include <cstdio>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <functional>

#include "bvh/bvh.hpp"
#include "bvh/binned_sah_builder.hpp"
#include "bvh/sweep_sah_builder.hpp"
#include "bvh/spatial_split_bvh_builder.hpp"
#include "bvh/locally_ordered_clustering_builder.hpp"
#include "bvh/linear_bvh_builder.hpp"
#include "bvh/parallel_reinsertion_optimizer.hpp"
#include "bvh/node_layout_optimizer.hpp"
#include "bvh/leaf_collapser.hpp"
#include "bvh/heuristic_primitive_splitter.hpp"
#include "bvh/hierarchy_refitter.hpp"
#include "bvh/single_ray_traverser.hpp"
#include "bvh/intersectors.hpp"
#include "bvh/triangle.hpp"


void reverseString(std::vector<char>& s) {
	int half = s.size() >> 1;
	int j = s.size() - 1;
	for (int i = 0; i < half; ++i, j--) {
		std::swap(s[i], s[j]);
	}
}

using Scalar = float;
using Vector3 = glm::tvec3<Scalar, glm::precision::aligned_highp>;// bvh::Vector3<Scalar>;
using Triangle = bvh::Triangle<Scalar>;
using Bvh = bvh::Bvh<Scalar>;
using Ray = bvh::Ray<Scalar>;

int main() {
	Bvh bvh;
	auto dahashybrowny = xxh::xxhash<32, char>("BOB");
	std::cout << dahashybrowny;
	std::vector<Triangle> triangles;
	triangles.emplace_back(
		Vector3(1.0, -1.0, 1.0),
		Vector3(1.0, 1.0, 1.0),
		Vector3(-1.0, 1.0, 1.0)
	);
	triangles.emplace_back(
		Vector3(1.0, -1.0, 1.0),
		Vector3(-1.0, -1.0, 1.0),
		Vector3(-1.0, 1.0, 1.0)
	);

	for (int i = 0; i < INT32_MAX; ++i) {

		bvh::SweepSahBuilder<Bvh> builder(bvh);
		auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(triangles.data(), triangles.size());
		auto global_bbox = bvh::compute_bounding_boxes_union(bboxes.get(), triangles.size());
		builder.build(global_bbox, bboxes.get(), centers.get(), triangles.size());

		// Intersect a ray with the data structure
		Ray ray(
			Vector3(0.0, 0.0, 0.0), // origin
			Vector3(0.0, 0.0, 1.0), // direction
			0.0,                    // minimum distance
			100.0                   // maximum distance
		);
		bvh::ClosestIntersector<false, Bvh, Triangle> intersector(bvh, triangles.data());
		bvh::SingleRayTraverser<Bvh> traverser(bvh);

		auto hit = traverser.traverse(ray, intersector);
		if (hit) {
			auto triangle_index = hit->primitive_index;
			auto intersection = hit->intersection;
			std::cout << "Hit triangle " << triangle_index << "\n"
				<< "distance: " << intersection.t << "\n"
				<< "u: " << intersection.u << "\n"
				<< "v: " << intersection.v << std::endl;
		}

	}
	//THE OLD WAY O LYFE
	/* 
	std::string dir = "../../../Assets/Levels/";
	glm::vec3 bob = glm::vec3(1);
	std::vector<Bounds> transforms = SceneLoader::LoadScene(dir, "Pong/Arena");
	KdTree Bob;
	Bob.build(transforms);

	BVHTree Bill;
	Bill.build(SplitMethod::SAH, TreeType::Recursive, transforms);
	Bill.flattenBVH();


	///_______________________________________________________________________________________________________________________________________________________________________
	//float tester = 0.f;
	//Bounds *midPtr = std::partition(&transforms[0], &transforms[transforms.size()-1] + 1, [tester](const Bounds& b) {
	//	return b.center.x < tester;
	//});
	//int mid = midPtr - &transforms[0];
	int size = transforms.size();
	int mid = size >> 1;
	int dim = 0;
	std::nth_element(&transforms[0], &transforms[mid], &transforms[size - 1] + 1, [dim](const Bounds& a, const Bounds& b) {
		return a.center[dim] < b.center[dim];
	});
	*/

	system("pause");
	return 0;
}