#ifndef _INTERSECT_MESH_GLSL
#define _INTERSECT_MESH_GLSL
#include "../structs.glsl"
#include "../layouts.glsl"

flool boundsIntersect(in Ray ray) {

	vec3 invDir = 1 / ray.d;
	vec3 invRay = invDir * ray.o;
	vec3 k = abs(invDir);

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

flool innerBoundsIntersect(in Ray ray, in vec3 center, in vec3 extents) {

	vec3 upperBounds = center + extents;
	vec3 lowerBounds = center - extents;

	vec3 invDir = 1 / ray.d;
	float tMin = FLT_MIN;
	float tMax = FLT_MAX;

	for (int i = 0; i < 3; ++i) {
		float t1 = (upperBounds[i] - ray.o[i]) * invDir[i];
		float t2 = (lowerBounds[i] - ray.o[i]) * invDir[i];

		tMin = max(tMin, min(t1, t2));
		tMax = min(tMax, max(t1, t2));
	}
	flool bob = flool(tMin, (tMax > max(tMin, 0.0)));
	return bob;
}

vec3 triNormal(Primitive prim, Face tri) {

	vec3 v0 = (prim.world * vec4(verts[tri.v[0]].pos, 1.f)).xyz;
	vec3 v1 = (prim.world * vec4(verts[tri.v[1]].pos, 1.f)).xyz;
	vec3 v2 = (prim.world * vec4(verts[tri.v[2]].pos, 1.f)).xyz;
	vec3 edge1 = v1 - v0;
	vec3 edge2 = v2 - v0;
	return normalize(cross(edge1, edge2));
}

flool triIntersect(Ray ray, Face tri) {

	vec3 edge1 = verts[tri.v[1]].pos - verts[tri.v[0]].pos;
	vec3 edge2 = verts[tri.v[2]].pos - verts[tri.v[0]].pos;
	vec3 h = cross(ray.d, edge2);
	float a = dot(edge1, h);
	if (a > -EPSILON && a < EPSILON)
		return flool(0, false);
	float f = 1 / a;
	vec3 s = ray.o - verts[tri.v[0]].pos;
	float u = f * dot(s, h);
	if (u < 0.f || u > 1.f)
		return flool(0, false);
	vec3 q = cross(s, edge1);
	float v = f * dot(ray.d, q);
	if (v < 0.f || u + v > 1.f)
		return flool(0, false);

	return flool(f * dot(edge2, q), true);
}

vec3 triIntersectUV(Ray ray, Face tri){
	vec3 edge1 = verts[tri.v[1]].pos - verts[tri.v[0]].pos;
	vec3 edge2 = verts[tri.v[2]].pos - verts[tri.v[0]].pos;
	vec3 h = cross(ray.d, edge2);
	float a = dot(edge1, h);
	if (a > -EPSILON && a < EPSILON)
		return vec3(-1.f);
	float f = 1 / a;
	vec3 s = ray.o - verts[tri.v[0]].pos;
	float u = f * dot(s, h);
	if (u < 0.f || u > 1.f)
		return vec3(-1.f);
	vec3 q = cross(s, edge1);
	float v = f * dot(ray.d, q);
	if (v < 0.f || u + v > 1.f)
		return vec3(-1.f);
	float t = f * dot(edge2, q);
	return vec3(t, u, v);
}

vec3 triNormalUV(Primitive prim, Face f, float u, float v){

	vec3 n0 = verts[f.v[0]].norm;
	vec3 n1 = verts[f.v[1]].norm;
	vec3 n2 = verts[f.v[2]].norm;

	vec3 lerp1 = mix(n0, n1, u);
	vec3 lerp2 = mix(n2, n1, u);

	mat4 temp2 =
	mat4(prim.extents.x, 0, 0, 0,
		 0, prim.extents.y, 0, 0,
		 0, 0, prim.extents.z, 0,
		0, 0, 0, 1);
	mat4 world = prim.world * temp2;
	return normalize(world * vec4(mix(lerp1, lerp2, v), 0.f)).xyz;
}

vec3 quadNormal(Primitive prim, Face f, float u, float v) {
	//vec3 n0 = (prim.world * vec4(verts[f.v[0]].norm, 1.f)).xyz;
	//vec3 n1 = (prim.world * vec4(verts[f.v[1]].norm, 1.f)).xyz;
	//vec3 n2 = (prim.world * vec4(verts[f.v[2]].norm, 1.f)).xyz;
	//vec3 n3 = (prim.world * vec4(verts[f.v[3]].norm, 1.f)).xyz;
	//vec3 edge1 = n1 - n0;
	//vec3 edge2 = n2 - n0;
	//return normalize(cross(edge1, edge2));

	vec3 n0 = verts[f.v[0]].norm;
	vec3 n1 = verts[f.v[1]].norm;
	vec3 n2 = verts[f.v[2]].norm;
	vec3 n3 = verts[f.v[3]].norm;

	//return (n0 + n1 + n2 + n3) / 4;

	vec3 lerp1 = mix(n0, n1, u);
	vec3 lerp2 = mix(n3, n2, u);
	//return (vec4(mix(lerp1, lerp2, v), 0.f) * prim.world).xyz;
	//vec4 temp = vec4(mix(lerp1, lerp2, v) , 0.f);
	//vec4 temp2 = vec4(prim.extents, 1.f);
	mat4 temp2 =
	mat4(prim.extents.x, 0, 0, 0,
		 0, prim.extents.y, 0, 0,
		 0, 0, prim.extents.z, 0,
		0, 0, 0, 1);
	mat4 world = prim.world * temp2;
	//return (temp2 * temp).xyz;
	//return (transpose(inverse(prim.world * temp2)) *  temp).xyz;
	return normalize(world * vec4(mix(lerp1, lerp2, v), 0.f)).xyz;
	//return (vec4(mix(lerp1, lerp2, v), 1.f) * prim.world).xyz;
	//return mix(lerp1, lerp2, v);
	//return vec3(0);
}

float copysign(float x, float y) {
	if ((x < 0 && y > 0) || (x > 0 && y < 0))
		return -x;
	return x;
}
vec4 quadIntersect(in Ray ray, in Face q) {
	vec3 q00 = verts[q.v[0]].pos, q10 = verts[q.v[1]].pos, q11 = verts[q.v[2]].pos, q01 = verts[q.v[3]].pos;
	vec3 e10 = q10 - q00;
	vec3 e11 = q11 - q10;
	vec3 e00 = q01 - q00;
	vec3 qn = cross(e10, (q01 - q11));
	//vec3 n;
	q00 -= ray.o;
	q10 -= ray.o;
	float a = dot(cross(q00, ray.d), e00);
	float c = dot(qn, ray.d);
	float b = dot(cross(q10, ray.d), e11);
	b -= a + c;
	float det = b * b - (4 * a*c);
	if (det < 0) return vec4(-1);
	det = sqrt(det);
	float u1, u2;
	float t = FLT_MAX, u, v;
	if (c == 0) {
		u1 = -a / b; u2 = -1;
	}
	else {
		//u1 = (-b - (det * sign(b))) / 2;
		u1 = (-b - copysign(det, b)) / 2;
		u2 = a / u1;
		u1 /= c;
	}
	if (0 <= u1 && u1 <= 1) {
		vec3 pa = mix(q00, q10, u1);
		vec3 pb = mix(e00, e11, u1);
		vec3 n = cross(ray.d, pb);
		det = dot(n, n);
		n = cross(n, pa);
		float t1 = dot(n, pb);
		float v1 = dot(n, ray.d);
		if (t1 > 0 && 0 <= v1 && v1 <= det) {
			t = t1 / det;
			u = u1;
			v = v1 / det;

		}

	}
	if (0 <= u2 && u2 <= 1) {
		vec3 pa = mix(q00, q10, u2);
		vec3 pb = mix(e00, e11, u2);
		vec3 n = cross(ray.d, pb);
		det = dot(n, n);
		n = cross(n, pa);
		float t2 = dot(n, pb) / det;
		float v2 = dot(n, ray.d);
		if (0 <= v2 && v2 <= det && t > t2 && t2 > 0) {
			t = t2;
			u = u2;
			v = v2 / det;
		}
	}
	//if (t == FLT_MAX) return vec4(-1.0);

	return vec4(t, u, v, 0);
	/*
	if (rtPotentialIntersection(t)) {
 // Fill the intersection structure irec.
 // Normal(s) for the closest hit will be normalized in a shader.
 float3 du = lerp(e10, q11 - q01, v);
 float3 dv = lerp(e00, e11, u);
 irec.geometric_normal = cross(du, dv);
 #if defined(SHADING_NORMALS)
 const float3* vn = patch.vertex_normals;
 irec.shading_normal = lerp(lerp(vn[0],vn[1],u),lerp(vn[3],vn[2],u),v);
 #else
 irec.shading_normal = irec.geometric_normal;
 #endif
 irec.texcoord = make_float3(u, v, 0);
 irec.id = prim_idx;
 rtReportIntersection(0u);
 }
 }


	*/
}
#endif
