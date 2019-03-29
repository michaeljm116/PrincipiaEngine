#ifndef _INTERSECT_SPHERE_GLSL
#define _INTERSECT_SPHERE_GLSL
#include "structs.glsl"

float sphereIntersect(in vec3 rayO, in vec3 rayD, in Primitive sphere)
{
	vec3 oc = rayO - sphere.center;
	float b = 2.0 * dot(oc, rayD);
	float c = dot(oc, oc) - sphere.extents.x*sphere.extents.x;
	float h = b * b - 4.0*c;
	if (h < 0.0)
	{
		return -1.0;
	}
	float t = (-b - sqrt(h)) / 2.0;

	return t;
}

vec3 sphereNormal(in vec3 pos, in Primitive sphere)
{
	return (pos - vec4(sphere.world[3]).xyz) / sphere.extents.x;
}

#endif