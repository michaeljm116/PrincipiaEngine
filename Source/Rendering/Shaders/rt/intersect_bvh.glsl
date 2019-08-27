#ifndef _INTERSECT_BVH_GLSL
#define _INTERSECT_BVH_GLSL
#include "structs.glsl"

bool bvhIntersect(vec3 rayO, vec3 rayD, in vec3 center, in vec3 extents) {

	vec3 upperBounds = center + extents;
	vec3 lowerBounds = center - extents;

	vec3 invDir = 1 / rayD;
	float tMin = FLT_MIN;
	float tMax = FLT_MAX;

	for (int i = 0; i < 3; ++i) {
		float t1 = (upperBounds[i] - rayO[i]) * invDir[i];
		float t2 = (lowerBounds[i] - rayO[i]) * invDir[i];

		tMin = max(tMin, min(t1, t2));
		tMax = min(tMax, max(t1, t2));
	}
	
	return (tMax > max(tMin, 0.0));
}

#endif