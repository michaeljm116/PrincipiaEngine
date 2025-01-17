#ifndef _INTERSECT_CYLINDER_GLSL
#define _INTERSECT_CYLINDER_GLSL
#include "../structs.glsl"

// Cylinder =========================================================
// cylinder defined by extremes pa and pb, and radious ra
vec4 cylinderIntersect(in Ray ray, in Primitive cyl) // extreme a, extreme b, radius
{
	vec3 pb = cyl.world[3].xyz;//cyl.bottom;
	vec3 pa = cyl.world[3].xyz;
	pb.y += cyl.extents.y;
	pa.y -= cyl.extents.y;
	pb = mat3(cyl.world) * pb;
	pa = mat3(cyl.world) * pa;

	float ra = cyl.extents.x;//radius;

	vec3 ca = pb - pa;

	vec3  oc = ray.o - pa;

	float caca = dot(ca, ca);
	float card = dot(ca, ray.d);
	float caoc = dot(ca, oc);

	float a = caca - card * card;
	float b = caca * dot(oc, ray.d) - caoc * card;
	float c = caca * dot(oc, oc) - caoc * caoc - ra * ra*caca;
	float h = b * b - a * c;
	if (h < 0.0) return vec4(-1.0);
	h = sqrt(h);
	float t = (-b - h) / a;

	// body
	float y = caoc + t * card;
	if (y > 0.0 && y < caca) return vec4(t, (oc + t * ray.d - ca * y / caca) / ra);

	// caps
	t = (((y < 0.0) ? 0.0 : caca) - caoc) / card;
	if (abs(b + a * t) < h)
	{
		return vec4(t, ca*sign(y) / caca);
	}

	return vec4(-1.0);
}


#endif
