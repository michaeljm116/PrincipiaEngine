#ifndef _INTERSECT_BOX_GLSL
#define _INTERSECT_BOX_GLSL
#include "structs.glsl"
vec4 boxIntersect(in vec3 ro, in vec3 rd, in Primitive box)
{
	// convert from ray to box space
	// currently 147/148
	mat4 invWorld = inverse(box.world);
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

vec3 boxTexture(in vec3 pos, in vec3 norm, in Primitive box, sampler2D t) {
	mat4 invWorld = inverse(box.world);
	vec3 div = 1 / box.extents * 0.5f;
	vec3 iPos = (vec4(pos, 1) * invWorld).xyz;
	vec3 iNorm = (vec4(norm, 0) * invWorld).xyz;

	vec4 xTxtr = texture(t, div.x * iPos.yz);
	vec4 yTxtr = texture(t, div.y * iPos.zx);
	vec4 zTxtr = texture(t, div.z * iPos.xy);

	vec3 ret = 
		abs(iNorm.x) * xTxtr.rgb * xTxtr.a +
		abs(iNorm.y) * yTxtr.rgb * yTxtr.a +
		abs(iNorm.z) * zTxtr.rgb * zTxtr.a;
	return ret;
}
#endif