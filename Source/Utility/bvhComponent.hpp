
//#include "componentIncludes.h"
#pragma once
#include <glm/glm.hpp>
#include <memory>
#include "helpers.h"
#include <assert.h>

#include <bvh/bvh.hpp>
#include <bvh/vector.hpp>
#include <bvh/bounding_box.hpp>
#include <bvh/ray.hpp>
#include <bvh/linear_bvh_builder.hpp>
#include <bvh/sweep_sah_builder.hpp>
#include <bvh/locally_ordered_clustering_builder.hpp>
#include "../Rendering/Components/renderComponents.hpp"

namespace Principia {
	enum class TreeType {
		Recursive,
		HLBVH
	};
	enum class SplitMethod {
		Middle, SAH, EqualsCounts
	};

	struct BVHBounds {
		glm::vec3 center;
		glm::vec3 extents;

		BVHBounds(glm::vec3 c, glm::vec3 e) : center(c), extents(e) {};
		BVHBounds() {};
		glm::vec3 max() {
			return center + extents;
		}
		glm::vec3 min() {
			return center - extents;
		}

		BVHBounds combine(BVHBounds b) {
			if (center.x == NAN) return b;
			//find the highest and the lowest x and y values
			glm::vec3 max = tulip::maxV(this->max(), b.max());
			glm::vec3 min = tulip::minV(this->min(), b.min());

			//center = halfway between the two, extents = max-center
			glm::vec3 c = (max + min) * 0.5f;
			glm::vec3 e = max - c;

			return BVHBounds(c, e);
		}

		BVHBounds combine(glm::vec3 c, glm::vec3 e) {
			glm::vec3 max = tulip::maxV(this->max(), (c + e));
			glm::vec3 min = tulip::minV(this->min(), (c - e));
			glm::vec3 ce = (max + min) * 0.5f;

			return BVHBounds(ce, max - ce);
		}

		float Offset(glm::vec3 c, int a) const {
			float ret = (c[a] - (center[a] - extents[a])) / (extents[a] * 2);
			return ret;
		}
		glm::vec3 Offset(glm::vec3 c) const {
			glm::vec3 ret = center;
			ret.x = Offset(c, 0);
			ret.y = Offset(c, 1);
			ret.z = Offset(c, 2);
			return ret;
		}

		float SurfaceArea() {
			glm::vec3 te = extents * 2.f;
			return 2 * (te.x * te.y + te.x * te.z + te.y * te.z);
		}
	};

	struct BVHBucket {
		int count = 0;
		BVHBounds bounds;// = BVHBounds(glm::vec3(0), glm::vec3(0));
	};

	struct BVHNode {
		BVHBounds bounds;
		BVHNode* children[2];
		int splitAxis;
		int firstPrimOffset;
		int nPrims;

		void initLeaf(int first, int n, const BVHBounds& b) {
			firstPrimOffset = first;
			nPrims = n;
			bounds = b;
			children[0] = children[1] = nullptr;
		}

		void initInterior(int axis, BVHNode* c0, BVHNode* c1) {
			children[0] = c0;
			children[1] = c1;
			//bounds = c0->bounds.union2D(c1->bounds);
			bounds = c0->bounds.combine(c1->bounds);
			splitAxis = axis;
			nPrims = 0;
		}
		
		void* operator new (size_t s, BVHNode* arena, size_t& curr) {
			curr += s;
			return arena + curr - s;
		}

		void* operator new (size_t s) {
			return new BVHNode();
		}
	};

	struct ssBVHNode {
		glm::vec3 upper;
		int offset;
		glm::vec3 lower;
		int numChildren;
	};

	struct MortonPrimitive {
		int primitive_index;
		uint32_t morton_code;
	};

	struct LBVHTreelet {
		size_t start_index, n_primitives;
		BVHNode* build_nodes;
	};

	struct BVHPrimitive {
		BVHPrimitive() {}
		BVHPrimitive(size_t prim_ind, const BVHBounds &_b) : primitive_index(prim_ind), bounds(_b) {}
		size_t primitive_index;
		BVHBounds bounds;
		glm::vec3 Centroid() const { return bounds.center; }
	};

	inline uint32_t LeftShift3(uint32_t x) {
		assert(x <= (1u << 10));
		if (x == (1 << 10))
			--x;
		x = (x | (x << 16)) & 0b00000011000000000000000011111111;
		// x = ---- --98 ---- ---- ---- ---- 7654 3210
		x = (x | (x << 8)) & 0b00000011000000001111000000001111;
		// x = ---- --98 ---- ---- 7654 ---- ---- 3210
		x = (x | (x << 4)) & 0b00000011000011000011000011000011;
		// x = ---- --98 ---- 76-- --54 ---- 32-- --10
		x = (x | (x << 2)) & 0b00001001001001001001001001001001;
		// x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
		return x;
	}

	inline uint32_t EncodeMorton3(float x, float y, float z) {
		assert(x >= 0);
		assert(y >= 0);
		assert(z >= 0);
		return (LeftShift3(z) << 2) | (LeftShift3(y) << 1) | LeftShift3(x);
	}

	static void RadixSort(std::vector<MortonPrimitive>* v) {
		std::vector<MortonPrimitive> tempVector(v->size());
		constexpr int bitsPerPass = 6;
		constexpr int nBits = 30;
		static_assert((nBits % bitsPerPass) == 0,
			"Radix sort bitsPerPass must evenly divide nBits");
		constexpr int nPasses = nBits / bitsPerPass;
		for (int pass = 0; pass < nPasses; ++pass) {
			// Perform one pass of radix sort, sorting _bitsPerPass_ bits
			int lowBit = pass * bitsPerPass;
			// Set in and out vector references for radix sort pass
			std::vector<MortonPrimitive>& in = (pass & 1) ? tempVector : *v;
			std::vector<MortonPrimitive>& out = (pass & 1) ? *v : tempVector;

			// Count number of zero bits in array for current radix sort bit
			constexpr int nBuckets = 1 << bitsPerPass;
			int bucketCount[nBuckets] = { 0 };
			constexpr int bitMask = (1 << bitsPerPass) - 1;
			for (const MortonPrimitive& mp : in) {
				int bucket = (mp.morton_code >> lowBit) & bitMask;
				assert(bucket >= 0);
				assert(bucket < nBuckets);
				++bucketCount[bucket];
			}

			// Compute starting index in output array for each bucket
			int outIndex[nBuckets];
			outIndex[0] = 0;
			for (int i = 1; i < nBuckets; ++i)
				outIndex[i] = outIndex[i - 1] + bucketCount[i - 1];

			// Store sorted values in output array
			for (const MortonPrimitive& mp : in) {
				int bucket = (mp.morton_code >> lowBit) & bitMask;
				out[outIndex[bucket]++] = mp;
			}
		}
		// Copy final result from _tempVector_, if needed
		if (nPasses & 1)
			std::swap(*v, tempVector);
	}
}

using Scalar = glm::float32_t;
using Vector3 = bvh::Vector3<Scalar>;
using Ray = bvh::Ray<Scalar>;
using Morton = uint32_t;
using Bvh = bvh::Bvh<Scalar>;

struct BvhPrim {
	BvhPrim(Principia::PrimitiveComponent* pc) {
		c = glm_to_bvh_Vec3(pc->center());
		e = glm_to_bvh_Vec3(pc->world[3]);
	}
	//struct Intersection {
	//	Scalar t;

	//	// Required member: returns the distance along the ray
	//	Scalar distance() const { return t; }
	//};

	// Required type: the floating-point type used
	using ScalarType = Scalar;
	// Required type: the intersection data type returned by the intersect() method
	//using IntersectionType = Intersection;

	BvhPrim() = default;

	// Required member: returns the center of the primitive
	Vector3 center() const {
		return c;// Vector3(0, 0, 0);
	}

	// Required member: returns a bounding box for the primitive (tighter is better)
	bvh::BoundingBox<Scalar> bounding_box() const {
		return e;// bvh::BoundingBox(Vector3(-1, -1, -1), Vector3(1, 1, 1));
	}

	// Required member: computes the intersection between a ray and the primitive
	//std::optional<Intersection> intersect(const Ray& ray) const {
	//	return std::make_optional<Intersection>(Intersection{ (ray.tmin + ray.tmax) * Scalar(0.5) });
	//}
	Vector3 c;
	bvh::BoundingBox<Scalar> e;

	bvh::Vector3<Scalar> glm_to_bvh_Vec3(glm::vec3 v) {
		return bvh::Vector3<Scalar>(v.x, v.y, v.z);
	}
	bvh::Vector3<Scalar> glm_to_bvh_Vec3(glm::vec4 v) {
		return bvh::Vector3<Scalar>(v.x, v.y, v.z);
	}
};

