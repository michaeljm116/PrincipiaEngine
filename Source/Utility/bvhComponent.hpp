//#include "componentIncludes.h"
#pragma once
#include <glm/glm.hpp>

namespace Principia {

	struct ssBVHNode {
		glm::vec3 upper;
		int offset;
		glm::vec3 lower;
		int numChildren;
	};
	struct BvhBounds {
		glm::vec3 lower = {};
		int _pad = 0;
		glm::vec3 upper = {};
		int _pad2 = 0;
		BvhBounds() {};
		BvhBounds(const glm::vec3& a, const glm::vec3& b) : lower(a), upper(b) {};
	};
	struct BvhNode {
		virtual float sah() = 0;
		virtual bool isLeaf() const = 0;
		inline BvhBounds merge(const BvhBounds& a, const BvhBounds& b) {
			return BvhBounds(glm::min(a.lower, b.lower), glm::max(a.upper, b.upper));
		}
		inline float area(const BvhBounds& a) {
			glm::vec3 te = a.upper - a.lower;
			return 2 * madd(te.x, (te.y + te.z), te.y * te.z);
		}
		__forceinline float madd(const float a, const float b, const float c) { return a * b + c; }
	};

	struct InnerBvhNode : public BvhNode
	{
		BvhBounds bounds[2];
		BvhNode* children[2];
		InnerBvhNode() {
			bounds[0] = bounds[1] = {};
			children[0] = children[1] = nullptr;
		}
		float sah() override {
			return 1.0f + (area(bounds[0]) * children[0]->sah() + area(bounds[1]) * children[1]->sah()) / area(merge(bounds[0], bounds[1]));
		}

		bool isLeaf() const override { return false; }
	};

	struct LeafBvhNode : public BvhNode
	{
		unsigned id;
		BvhBounds bounds;
		LeafBvhNode(unsigned id, const BvhBounds& bounds) : id(id), bounds(bounds) {}
		float sah() { return 1.0f; }
		bool isLeaf() const override { return true; }
	};
};

