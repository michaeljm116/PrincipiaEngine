#ifndef TRAVERSE_GLSL
#define TRAVERSE_GLSL
#include "helpers.glsl"

void traverse_blas(Ray ray, Ray inv_ray, Primitive prim, inout HitInfo hit)
{
    for (int f = prim.startIndex; f < prim.endIndex; ++f)
    {
        vec4 t_quad = quadIntersect(inv_ray, faces[f]);
        if ((t_quad.x > 0) && (t_quad.x > EPSILON) && (t_quad.x < ray.t))
        {
            ray.t = t_quad.x;
            hit.t = t_quad.x;
            hit.normal.x = t_quad.y;
            hit.normal.y = t_quad.z;
        }
    }
}

bool quick_traverse_blas(Ray ray, Ray inv_ray, Primitive prim, inout HitInfo hit){
    for (int f = prim.startIndex; f < prim.endIndex; ++f)
    {
        vec4 t_quad = quadIntersect(inv_ray, faces[f]);
        if(is_hit(t_quad.x, ray.t)) return true;
    }
    return false;
}

HitInfo traverse(inout Ray ray, in int offset)
{
    int prim_id = primitives[offset].id;
    Primitive prim = primitives[offset];
    uint type = hit_type(prim_id);
    HitInfo hit = HitInfo(MAXLEN, vec3(0), type, offset, -1, offset);

    switch (type) {
        case TYPE_SPHERE:
        {
            hit.t = sphereIntersect(ray, prim);
            ray.t = check_hit(hit.t, ray.t);
            break;
        }
        case TYPE_BOX:
        {
            set_hit_with_normal(hit, boxIntersect(ray, prim));
            set_ray_if_hit(hit.t, ray);
            break;
        }
        case TYPE_CYLINDER:
        {
            set_hit_with_normal(hit, cylinderIntersect(ray, prim));
            set_ray_if_hit(hit.t, ray);
            break;
        }
        case TYPE_PLANE:
        {
            hit.t = planeIntersect(ray, prim);
            ray.t = check_hit(hit.t, ray.t);
            break;
        }
        case TYPE_DISK:
        {
            hit.t = diskIntersect(ray, prim);
            ray.t = check_hit(hit.t, ray.t);
            break;
        }
        case TYPE_QUAD:
        {
            vec2 uv = vec2(0);
            set_hit_with_normal(hit, quadTexIntersect(ray, prim, uv));
            set_ray_if_hit(hit.t, ray);
            break;
        }
        case TYPE_MESH:
        {
            Ray inv_ray = get_inverse_ray(ray, prim);
            flool t_mesh = boundsIntersect(inv_ray);
            if (!t_mesh.b) break;
            if ((t_mesh.t > EPSILON) && (t_mesh.t < ray.t)) {
                traverse_blas(ray, inv_ray, prim, hit);
            }
            break;
        }
        default:
        {
            break;
        }
    }
    return hit;
}

// This is mostly for shadows, if it hits ANYTHING AT ALL then IMMEDIATELY EXIT
float quick_traverse(inout Ray ray, in int offset, in float val)
{
    int prim_id = primitives[offset].id;
    Primitive prim = primitives[offset];
    uint type = hit_type(prim_id);
    HitInfo hit = HitInfo(MAXLEN, vec3(0), type, offset, -1, offset);

    switch (type)
    {
        case TYPE_SPHERE:
        {
            hit.t = sphereIntersect(ray, prim);
            if(is_hit(hit.t, ray.t)) return val;
            break;
        }
        case TYPE_BOX:
        {
            set_hit_with_normal(hit, boxIntersect(ray, prim));
            if(is_hit(hit.t, ray.t)) return val;
            break;
        }
        case TYPE_CYLINDER:
        {
            set_hit_with_normal(hit, cylinderIntersect(ray, prim));
            if(is_hit(hit.t, ray.t)) return val;
            break;
        }
        case TYPE_PLANE:
        {
            hit.t = planeIntersect(ray, prim);
            if(is_hit(hit.t, ray.t)) return val;
            break;
        }
        case TYPE_DISK:
        {
            hit.t = diskIntersect(ray, prim);
            if(is_hit(hit.t, ray.t)) return val;
            break;
        }
        case TYPE_QUAD:
        {
            vec2 uv = vec2(0);
            set_hit_with_normal(hit, quadTexIntersect(ray, prim, uv));
            if(is_hit(hit.t, ray.t)) return val;
            break;
        }
        case TYPE_MESH:
        {
            Ray inv_ray = get_inverse_ray(ray, prim);
            flool t_mesh = boundsIntersect(inv_ray);
            if (!t_mesh.b) break;
            if ((t_mesh.t > EPSILON) && (t_mesh.t < ray.t)) {
                if(quick_traverse_blas(ray, inv_ray, prim, hit)) return val;
            }
            break;
        }
        default:
        {
            break;
        }
    }
    return -val;
}

#endif
