#ifndef _INTERSECT_MESH_GLSL
#define _INTERSECT_MESH_GLSL
#include "structs.glsl"

flool boundsIntersect(vec3 rayO, vec3 rayD, Primitive box) {

	vec3 invDir = 1 / rayD;
	vec3 invRay = invDir * rayO;
	vec3 k = abs(invDir) * box.extents;

	float tMin = FLT_MIN;
	float tMax = FLT_MAX;

	vec3 t1 = -invRay - k;
	vec3 t2 = -invRay + k;

	for (int i = 0; i < 3; ++i) {
		tMin = max(tMin, min(t1[i], t2[i]));
		tMax = min(tMax, max(t1[i], t2[i]));
	}
	flool bob = flool(tMin, (tMax > max(tMin, 0.0)));
	return bob;
}

#endif