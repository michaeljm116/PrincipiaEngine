#ifndef _INTERSECT_MESH_GLSL
#define _INTERSECT_MESH_GLSL
#include "structs.glsl"
#include "layouts.glsl"

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

vec3 triNormal(in TriangleIndex tri) {
	vec3 edge1 = verts[tri.v1].pos - verts[tri.v0].pos;
	vec3 edge2 = verts[tri.v2].pos - verts[tri.v0].pos;

	return normalize(cross(edge1, edge2));
}
// verts[tri.v0].pos
flool triIntersect(vec3 rayO, vec3 rayD, TriangleIndex tri) {

	vec3 edge1 = verts[tri.v1].pos - verts[tri.v0].pos;
	vec3 edge2 = verts[tri.v2].pos - verts[tri.v0].pos;
	vec3 h = cross(rayD, edge2);
	float a = dot(edge1, h);
	if (a > -EPSILON && a < EPSILON)
		return flool(0, false);
	float f = 1 / a;
	vec3 s = rayO - verts[tri.v0].pos;
	float u = f * dot(s, h);
	if (u < 0.f || u > 1.f)
		return flool(0, false);
	vec3 q = cross(s, edge1);
	float v = f * dot(rayD, q);
	if (v < 0.f || u + v > 1.f)
		return flool(0, false);

	return flool(f * dot(edge2, q), true);
}

vec4 quadIntersect(in vec3 rO, in vec3 rd, in QuadIndex q) {
	vec3 q00 = verts[q.v[0]].pos, q10 = verts[q.v[1]].pos, q11 = verts[q.v[2]].pos, q01 = verts[q.v[3]].pos;
	vec3 e10 = q10 - q00;
	vec3 e11 = q11 - q10;
	vec3 e00 = q01 - q00;
	vec3 qn = cross(e10, (q01 - q11));
	vec3 n;
	q00 -= rO;
	q10 -= rO;
	float a = dot(cross(q00, rd), e00);
	float c = dot(qn, rd);
	float b = dot(cross(q10, rd), e11);
	b -= a + c;
	float det = b * b - 4 * a*c;
	if (det < 0) return vec4(-1.f);
	det = sqrt(det);
	float u1, u2;
	float t = FLT_MAX;// , u, v;
	if (c == 0) {
		u1 = -a / b; u2 = -1;
	}
	else {
		u1 = (-b - (det * sign(b))) / 2;
		u2 = a / u1;
		u1 /= c;
	}
	if (0 <= u1 && u1 <= 1) {
		vec3 pa = mix(q00, q10, u1);
		vec3 pb = mix(e00, e11, u1);
		vec3 n = cross(rd, pb);
		det = dot(n, n);
		n = cross(n, pa);
		float t1 = dot(n, pb);
		float v1 = dot(n, rd);
		if (t1 > 0 && 0 <= v1 && v1 <= det) {
			t = t1 / det;// u = u1; v = v1 / det; 

		}

	}
	if (0 <= u2 && u2 <= 1) {
		vec3 pa = mix(q00, q10, u2);
		vec3 pb = mix(e00, e11, u2);
		vec3 n = cross(rd, pb);
		det = dot(n, n);
		n = cross(n, pa);
		float t2 = dot(n, pb) / det;
		float v2 = dot(n, rd);
		if (0 <= v2 && v2 <= det && t > t2 && t2 > 0) {
			t = t2; //u = u2; v = v2 / det;	
		}
	}
	if (t == FLT_MAX) return vec4(-1);
	return vec4(t, n);
}
#endif