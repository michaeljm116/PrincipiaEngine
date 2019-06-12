#ifndef _INTERSECT_BOX_GLSL
#define _INTERSECT_BOX_GLSL
#include "structs.glsl"
vec4 boxIntersect(in vec3 ro, in vec3 rd, in Primitive box)
{
	// convert from ray to box space
	// currently 147/148
	mat4 invWorld = box.invWorld;// inverse(box.world);
	vec3 rdd = (invWorld*vec4(rd, 0.0)).xyz;
	vec3 roo = (invWorld*vec4(ro, 1.0)).xyz;

	// ray-box intersection in box space
	vec3 m = 1.0 / rdd;
	vec3 n = m * roo;
	vec3 k = abs(m)*box.extents;

	vec3 t1 = -n - k;
	vec3 t2 = -n + k;

	float tN = max(max(t1.x, t1.y), t1.z);
	float tF = min(min(t2.x, t2.y), t2.z);
	if (tN > tF || tF < 0.0) return vec4(-1.0);

	//problem: you're calculatinog the normals before knowing if its the closest
	//should instead just send back the tn value, rdd and t1
	//current fps = 73.1ms, 13.7 fps
	//return rdd, t1, sign 3 + 3 + 
	vec3 nor = -sign(rdd)*step(t1.yzx, t1.xyz)*step(t1.zxy, t1.xyz);

	// convert to ray space

	nor = (box.world * vec4(nor, 0.0)).xyz;

	return vec4(tN, nor);
}
#endif