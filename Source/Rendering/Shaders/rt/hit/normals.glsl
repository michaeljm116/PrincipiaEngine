#ifndef NORMALS_GLSL
#define NORMALS_GLSL
#include "../structs.glsl"
#include "../constants.glsl"

vec3 sphere_normal(in vec3 pos, in Primitive sphere)
{
	return (pos - vec4(sphere.world[3]).xyz) / sphere.extents.x;
}

vec3 disk_normal(){
    return vec3(0,1,0);
}

vec3 quad_normal(Primitive prim, Face f, float u, float v) {
	vec3 n0 = verts[f.v[0]].norm;
	vec3 n1 = verts[f.v[1]].norm;
	vec3 n2 = verts[f.v[2]].norm;
	vec3 n3 = verts[f.v[3]].norm;

	vec3 lerp1 = mix(n0, n1, u);
	vec3 lerp2 = mix(n3, n2, u);
	mat4 temp2 =
	mat4(prim.extents.x, 0, 0, 0,
		 0, prim.extents.y, 0, 0,
		 0, 0, prim.extents.z, 0,
		0, 0, 0, 1);
	mat4 world = prim.world * temp2;
	return normalize(world * vec4(mix(lerp1, lerp2, v), 0.f)).xyz;
}

void set_normals(inout HitInfo info, in vec3 ray_pos){
    switch (info.prim_type) {
        case TYPE_DISK:
        {
            info.normal = disk_normal();
            break;
        }
        case TYPE_SPHERE:
        {
            info.normal = sphereNormal(ray_pos, primitives[info.face_id]);
            break;
        }
        case TYPE_MESH:
        {
            info.normal = quad_normal(primitives[info.prim_id], faces[info.face_id], info.normal.x, info.normal.y);
            break;
        }
        default:
            break;
    }
}
#endif
