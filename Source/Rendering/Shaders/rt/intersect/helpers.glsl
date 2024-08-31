#ifndef HELPERS_GLSL
#define HELPERS_GLSL

#include "../constants.glsl"
#include "../structs.glsl"

float check_hit(float t, float ray_t) {
    if ((t > EPSILON) && (t < ray_t)) return t;
    else return ray_t;
}
bool is_hit(float t, float ray_t){
    return ((t > EPSILON) && (t < ray_t));
}

void set_ray_if_hit(float t, inout Ray ray) {
    if ((t > EPSILON) && (t < ray.t)) ray.t = t;
}
uint hit_type(int prim_id) {
    switch (prim_id) {
        case -1:
        return TYPE_SPHERE;
        case -2:
        return TYPE_BOX;
        case -3:
        return TYPE_CYLINDER;
        case -4:
        return TYPE_PLANE;
        case -5:
        return TYPE_DISK;
        case -6:
        return TYPE_QUAD;
        default:
        return TYPE_MESH;
    }
}
void set_hit_with_normal(inout HitInfo hit, vec4 t_and_norm)
{
    hit.t = t_and_norm.x;
    hit.normal = t_and_norm.yzw;
}
Ray get_inverse_ray(Ray ray, Primitive prim)
{
    mat4 inv_world = inverse(prim.world);
    vec3 inv_origin = (inv_world * vec4(ray.o, 1.0)).xyz;
    vec3 inv_distance = (inv_world * vec4(ray.d, 0.0)).xyz;
    return Ray(inv_origin, FLT_MAX, inv_distance, FLT_MAX);
}

#endif
