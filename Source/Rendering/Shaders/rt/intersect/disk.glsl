#ifndef _INTERSECT_DISK_GLSL
#define _INTERSECT_DISK_GLSL
#include "plane.glsl"

float diskIntersect(in Ray ray, in Primitive disk)
{
	float radius = disk.extents.x;
	vec3 n = vec3(0, 1, 0);
	vec3 p0 = disk.world[3].xyz;

	float t = planeIntersect(ray, disk);
	if (t != 0.0f) {
		vec3 p = ray.o + ray.d * t;
		vec3 v = p - p0;
		float d2 = dot(v, v);
		float r2 = radius * radius;
		//if (sqrt(d2) <= radius)
		if (d2 <= r2)
			return t;
		// or you can use the following optimisation (and precompute radius^2)
		// return d2 <= radius2; // where radius2 = radius * radius
	}

	return 0.0f;
}

#endif
