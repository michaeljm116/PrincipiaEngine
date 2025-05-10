#pragma once
#include "Components/collisionComponent.h"
#include <glm/gtx/norm.hpp>
#include "../Utility/helpers.h"
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

			/*
			a = box
			b = sphere
			sphere.x = sphere.x < boxmin.x ? boxmin.x : sphere.x = sphere.x
			spehre.x = sphere.x > boxmax.x ? boxmax.x : sphere.x = boxmax.x
			sphere.y = sphere.y < boxmin.y ? boxmin.y : sphere.y = sphere.y
			sphere.y = sphere.y > boxmax.y ? boxmax.y : sphere.y = sphere.y

			closest point = boxmax.x, sphere.y
			*/
			return result;
		}

		typedef bool(*ct)(const CollisionComponent* const, const CollisionComponent* const, CollisionData&, CollisionData&);
		bool SphereSphere(const CollisionComponent* const a, const CollisionComponent* const b, CollisionData& aD, CollisionData& bD) {
			float distsqrd = static_cast<float>(pow(a->position.x - b->position.x, 2) + pow(a->position.y - b->position.y, 2) + pow(a->position.z - b->position.z, 2));
			if (distsqrd < static_cast<float>(pow(a->extents.x + b->extents.x, 2))) {
				//A collision has occured, so return the normal and collision point
				aD.normal = b->position - a->position;
				aD.normal /= sqrt(aD.normal);
				bD.normal = -aD.normal;

				aD.colpoint = a->position + (a->extents.x * aD.normal);
				bD.colpoint = aD.colpoint;

				return true;
			}
			else return false;
		}
		bool SphereBox(const CollisionComponent* const a, const CollisionComponent* const b, CollisionData& aD, CollisionData& bD) {
			glm::vec3 closestPoint = ClosestPoint(*b, *a);
			float distSq = glm::distance2(a->position, closestPoint);
			glm::vec3 dist = a->position - closestPoint;
			//n = dist / sqrt(dist);
			float radSq = a->extents.x * a->extents.x;
			return distSq < radSq;
		};
		bool SphereCapsule(const CollisionComponent* const a, const CollisionComponent* const b, CollisionData& aD, CollisionData& bD) {return false;};
		bool SphereOther(const CollisionComponent* const a, const CollisionComponent* const b, CollisionData& aD, CollisionData& bD) {return false;};
		bool BoxSphere(const CollisionComponent* const a, const CollisionComponent* const b, CollisionData& aD, CollisionData& bD) {
			glm::vec3 closestPoint = ClosestPoint(*a, *b);
			float distSq = glm::distance2(b->position, closestPoint);
			glm::vec3 dist = b->position - closestPoint;
			//n = dist / sqrt(dist);
			float radSq = b->extents.x * b->extents.x;
			return distSq < radSq;
		};
		bool BoxBox(const CollisionComponent* const a, const CollisionComponent* const b, CollisionData& aD, CollisionData& bD) {

			glm::vec3 extents = a->extents + b->extents;
			glm::vec3 distance = b->position - a->position;
			//glm::vec3 absdist = abs(distance);
			glm::vec3 res = extents - abs(distance);

			for (int i = 0; i < 3; ++i) {
				if (res[i] <= 0)
					return false;
			}

			//A Collision has occured so find the normal and collision point
			aD.normal = glm::vec3(0);
			if ((b->position.x > (a->position.x - a->extents.x)) || (b->position.x <= (a->position.x + a->extents.x)))
				aD.normal.z = static_cast<float>(sign(distance.z));
			else
				aD.normal.x = static_cast<float>(sign(distance.x));
			//res.x < res.z ? aD.normal.x = sign(distance.x) : aD.normal.z = sign(distance.z);
			bD.normal = -aD.normal;

			//if two (aabb) boxes collide there is no real collision "point" so the axis of the normal is really all that matters
			aD.colpoint = b->position + (bD.normal * b->extents);
			bD.colpoint = a->position + (aD.normal * a->extents);

			return true;
		};
		bool BoxCapsule(const CollisionComponent* const a, const CollisionComponent* const b, CollisionData& aD, CollisionData& bD) {return false;};
		bool BoxOther(const CollisionComponent* const a, const CollisionComponent* const b, CollisionData& aD, CollisionData& bD) {return false;};
		bool CapsuleSphere(const CollisionComponent* const a, const CollisionComponent* const b, CollisionData& aD, CollisionData& bD) {return false;};
		bool CapsuleBox(const CollisionComponent* const a, const CollisionComponent* const b, CollisionData& aD, CollisionData& bD) {return false;};
		bool CapsuleCapsule(const CollisionComponent* const a, const CollisionComponent* const b, CollisionData& aD, CollisionData& bD) {return false;};

		//Converts the type to a bitwise operation
		inline int Convert(const CollisionType& a, const CollisionType& b) {
			int res = (int)a;
			res |= ((int)b << 2);
			return res;
		}
		
		static ct ColTests[]{ SphereSphere, SphereBox, SphereCapsule, SphereOther, BoxSphere, BoxBox, BoxCapsule, BoxOther, CapsuleSphere, CapsuleBox, CapsuleCapsule };
	}
}