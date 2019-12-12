#pragma once
#include "Components/collisionComponent.h"
#include <glm/gtx/norm.hpp>
/*--------------------------------------------
Basically you take two 2bit thingies and turn it into a 4bit thingy
then use that 4bit thingy to select the collision test you want

sphere = 0 box = 1 Capsule = 2 unknown = 3
0 | 00 00 sphere sphere
1 | 00 01 sphere box
2 | 00 10 sphere Capsule
3 | 00 11 sphere unknown
4 | 01 00 box sphere
5 | 01 01 box box
6 | 01 10 box Capsule
7 | 01 11 box unknown
8 | 10 00 Capsule sphere
9 | 10 01 Capsule box
10 | 10 10 Capsule Capsule
--------------------------------------------*/

namespace Principia {
	namespace CollisionTests {

		inline glm::vec3 ClosestPoint(const CollisionComponent& a, const CollisionComponent& b) {
			glm::vec3 result = b.position;
			glm::vec3 min = a.position - a.extents;
			glm::vec3 max = a.position + a.extents;
			for (int i = 0; i < 3; ++i) {
				result[i] = (result[i] < min[i]) ? min[i] : result[i];
				result[i] = (result[i] > max[i]) ? max[i] : result[i];
			}
			return result;
		}

		typedef bool(*ct)(const CollisionComponent* const, const CollisionComponent* const, glm::vec3&);
		inline bool SphereSphere(const CollisionComponent* const a, const CollisionComponent* const b, glm::vec3& p) {
			float dist = sqrt(pow(a->position.x - b->position.x, 2) + pow(a->position.y - b->position.y, 2) + pow(a->position.z - b->position.z, 2));
			if (dist < a->extents.x + b->extents.x)
				return true;
			else return false;
		}
		bool SphereBox(const CollisionComponent* const a, const CollisionComponent* const b, glm::vec3& p) {
			glm::vec3 closestPoint = ClosestPoint(*b, *a);
			float distSq = glm::distance2(a->position, closestPoint);
			float radSq = a->extents.x * a->extents.x;
			return distSq < radSq;
		};
		bool SphereCapsule(const CollisionComponent* const a, const CollisionComponent* const b, glm::vec3& p) {return false;};
		bool SphereOther(const CollisionComponent* const a, const CollisionComponent* const b, glm::vec3& p) {return false;};
		bool BoxSphere(const CollisionComponent* const a, const CollisionComponent* const b, glm::vec3& p) {
			glm::vec3 closestPoint = ClosestPoint(*a, *b);
			float distSq = glm::distance2(b->position, closestPoint);
			float radSq = b->extents.x * b->extents.x;
			return distSq < radSq;
		};
		bool BoxBox(const CollisionComponent* const a, const CollisionComponent* const b, glm::vec3& p) {
			glm::vec3 amin = a->position - a->extents;
			glm::vec3 amax = a->position + a->extents;
			glm::vec3 bmin = b->position - b->extents;
			glm::vec3 bmax = b->position + b->extents;

			for (int i = 0; i < 3; ++i) {
				if ((amax[i] < bmin[i]) || (amin[i] > bmax[i]))
					return false;
			}

			return true;
		};
		bool BoxCapsule(const CollisionComponent* const a, const CollisionComponent* const b, glm::vec3& p) {return false;};
		bool BoxOther(const CollisionComponent* const a, const CollisionComponent* const b, glm::vec3& p) {return false;};
		bool CapsuleSphere(const CollisionComponent* const a, const CollisionComponent* const b, glm::vec3& p) {return false;};
		bool CapsuleBox(const CollisionComponent* const a, const CollisionComponent* const b, glm::vec3& p) {return false;};
		bool CapsuleCapsule(const CollisionComponent* const a, const CollisionComponent* const b, glm::vec3& p) {return false;};

		inline int Convert(const CollisionType& a, const CollisionType& b) {
			int res = (int)a;
			res |= ((int)b << 2);
			return res;
		}
		
		static ct ColTests[]{ SphereSphere, SphereBox, SphereCapsule, SphereOther, BoxSphere, BoxBox, BoxCapsule, BoxOther, CapsuleSphere, CapsuleBox, CapsuleCapsule };
	}
}