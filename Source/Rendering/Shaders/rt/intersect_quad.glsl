#ifndef _INTERSECT_QUAD_GLSL
#define _INTERSECT_QUAD_GLSL
#include "structs.glsl"

vec4 diskIntersect(in vec3 rO, in vec3 rd, in QuadIndex q, in Vert[] verts)
{
	vec3 q00 = verts[q.v[0]], q10 = verts[q.v[1]], q11 = verts[q.v[2]], q01 = verts[q.v[3]];
	vec3 e10 = q10 - q00;
	vec3 e11 = q11 - q10; 
	vec3 e00 = q01 - q00; 
	vec3 qn = q.v[4];
	q00 -= rO; 
	q10 -= rO;
	float a = dot(cross(q00, rd), e00);
	float c = dot(qn, rd);
	float b = dot(cross(q10, rd), e11); 
	b -= a + c; 
	float det = b * b - 4 * a*c;
	if (det < 0) return; 
	det = sqrt(det); 
	float u1, u2; 
	float t = FLT_MAX;// , u, v;
	if (c == 0) {
		u1 = -a / b; u2 = -1;
	}
	else {
		u1 = (-b - sign(det, b)) / 2; 
		u2 = a / u1; 
		u1 /= c;
	}
	if (0 <= u1 && u1 <= 1) { 
		vec3 pa = lerp(q00, q10, u1); 
		vec3 pb = lerp(e00, e11, u1);
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
		vec3 pa = lerp(q00, q10, u2); 
		vec3 pb = lerp(e00, e11, u2); 
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