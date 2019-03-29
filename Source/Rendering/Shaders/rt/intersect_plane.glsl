#ifndef _INTERSECT_PLANE_GLSL
#define _INTERSECT_PLANE_GLSL
#include "structs.glsl"

float planeIntersect(in vec3 rayO, in vec3 rayD, in Primitive plane)
{
	vec3 normal = vec3(0, 1, 0);
	float d = dot(rayD, normal);

	if (d > EPSILON)
		return 0.0;

	//float t = -(plane.distance + dot(rayO, normal)) / d;
	float t = dot(plane.center - rayO, normal) / d;

	if (t < 0.0)
		return 0.0;

	return t;
}


#endif