
//#include "componentIncludes.h"
#pragma once
#include <glm/glm.hpp>
#include <memory>
#include "helpers.h"

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
		std::shared_ptr<BVHNode> children[2];
		int splitAxis;
		int firstPrimOffset;
		int nPrims;

		void initLeaf(int first, int n, const BVHBounds& b) {
			firstPrimOffset = first;
			nPrims = n;
			bounds = b;
			children[0] = children[1] = nullptr;
		}

		void initInterior(int axis, std::shared_ptr<BVHNode> c0, std::shared_ptr<BVHNode> c1) {
			children[0] = c0;
			children[1] = c1;
			//bounds = c0->bounds.union2D(c1->bounds);
			bounds = c0->bounds.combine(c1->bounds);
			splitAxis = axis;
			nPrims = 0;
		}
	};

	struct ssBVHNode {
		glm::vec3 upper;
		int offset;
		glm::vec3 lower;
		int numChildren;
	};
}