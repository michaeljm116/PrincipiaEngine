#ifndef _INTERSECT_PLANE_GLSL
#define _INTERSECT_PLANE_GLSL
#include "../structs.glsl"

float planeIntersect(in Ray ray, in Primitive plane)
{
	vec3 normal = vec3(0, 1, 0);
	float d = dot(ray.d, normal);

	if (d > EPSILON)
		return 0.0;

	//float t = -(plane.distance + dot(rayO, normal)) / d;
	float t = dot(vec3(plane.world[3].xyz) - ray.o, normal) / d;

	if (t < 0.0)
		return 0.0;

	return t;
}


#endif
