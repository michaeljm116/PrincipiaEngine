#ifndef _MAIN_INTERSECT_GLSL
#define _MAIN_INTERSECT_GLSL

#include "../intersect_plane.glsl"
#include "../intersect_box.glsl"
#include "../intersect_sphere.glsl"
#include "../intersect_mesh.glsl"
#include "../intersect_cylinder.glsl"
#include "../intersect_disk.glsl"
#include "../intersect_bvh.glsl"
#include "../structs.glsl"
#include "../layouts.glsl"
#include "constants-intersect.glsl"


void insert_hit_info(HitInfo hits[MAX_INTERSECTIONS], HitInfo hit) {

    // If the first slot is empty, place the new hit and return
    if (hits[0].prim_type == 0) {
        hits[0] = hit;
        return;
    }

    // Find the correct position for the new hit based on its 't' value
    for (uint i = 0; i < MAX_INTERSECTIONS; ++i) {
        // If the current hit's 't' value is larger, insert here
        if (hit.t < hits[i].t || hits[i].prim_type == 0) {
            // Shift the array to make room for the new hit
            for (uint j = MAX_INTERSECTIONS - 1; j > i; --j) {
                hits[j] = hits[j - 1];
            }
            // Insert the new hit in the correct position
            hits[i] = hit;
            return;
        }
    }
}


HitInfo intersect_closest(Ray ray)
{
    // Set up
    HitInfo info = HitInfo(MAXLEN, vec3(0), 0, -1, -1, -1);
    return info;
/*    int stack[16];
    int sp = 0; //stack pointer
    stack[0] = 0;
    vec3 invDir = 1 / ray.d;

    // Main Intersection Loop
    while(sp > -1)
    {
        int offset = stack[sp];
        BVHNode node = bvhNodes[offset];
        sp--;
        #ifdef DEBUGLINES
        if (bvhBoundsIntersect(ray, invDir, bvhNodes[offset])) {
            return HitInfo(MAXLEN, vec3(0), 0, 0, 0, offset);
        }
        #endif

        //Hit primitive if no children
        if(node.numChildren == 0)
        {
            HitInfo temp = traverse(ray, node.offset);
            if(temp.t < info.t) info = temp;
        }
        else
        {
            offset = offset + 1;
            if (mbvhIntersect(ray, invDir, bvhNodes[offset])) {
                sp++;
                stack[sp] = offset;
            }
            if (node.offset < bvhNodes.length()) {
                if (mbvhIntersect(ray, invDir, bvhNodes[node.offset])) {
                    sp++;
                    stack[sp] = node.offset;
                }
            }
        }

    }
    // Return
    */
    return info;
}

float check_hit(float t, float ray_t){
    if ((t > EPSILON) && (t < ray_t)) return t;
    else return ray_t;
}

void set_ray_if_hit(float t, inout Ray ray){
    if((t > EPSILON) && (t < ray.t)) ray.t = t;
}

uint hit_type(int prim_id){
    switch(prim_id){
        case -1: return TYPE_SPHERE;
        case -2: return TYPE_BOX;
        case -3: return TYPE_CYLINDER;
        case -4: return TYPE_PLANE;
        case -5: return TYPE_DISK;
        case -6: return TYPE_QUAD;
        default: return TYPE_MESH;
    }
}

void set_hit_with_normal(inout HitInfo hit, vec4 t_and_norm)
{
    hit.t = t_and_norm.x;
    hit.normal = t_and_norm.yzw;
}

HitInfo traverse(inout Ray ray, in int offset)
{
    int prim_id = primitives[offset].id;
    Primitive prim = primitives[offset];
    uint type = hit_type(prim_id);
    HitInfo hit = HitInfo(MAXLEN, vec3(0), type, offset, -1, offset);

    switch(type){
        case TYPE_SPHERE:{
            hit.t = sphereIntersect(ray,prim);
            ray.t = check_hit(hit.t, ray.t);
            break;
        }
        case TYPE_BOX:{
            set_hit_with_normal(hit,boxIntersect(ray,prim));
            set_ray_if_hit(hit.t, ray);
            break;
        }
        case TYPE_CYLINDER:{
            set_hit_with_normal(hit, cylinderIntersect(ray,prim));
            set_ray_if_hit(hit.t, ray);
            break;
        }
        case TYPE_PLANE:{
            hit.t = planeIntersect(ray, prim);
            ray.t = check_hit(hit.t, ray.t);
            break;
        }
        case TYPE_DISK:{
            hit.t = diskIntersect(ray, prim);
            ray.t = check_hit(hit.t, ray.t);
        }
        case TYPE_QUAD:{
            vec2 uv = vec2(0);
            set_hit_with_normal(hit, quadTexIntersect(ray, prim, uv));
            set_ray_if_hit(hit.t, ray);
        }
        case TYPE_MESH:{
            mat4 inv_world = inverse(prim.world);
            vec3 inv_origin = (inv_world * vec4(ray.o, 1.0)).xyz;
            vec3 inv_distance = (inv_world * vec4(ray.d, 0.0)).xyz;
            Ray inv_ray = Ray(inv_origin, FLT_MAX, inv_distance, FLT_MAX);
            flool t_mesh = boundsIntersect(inv_ray);
            if(!t_mesh.b)break;
            if((t_mesh.t > EPSILON) && (t_mesh.t < ray.t)){
                // Begin BLAS Intersection test,
                // but we dont have a BLAS right now
                // so.... Do it linearly
                int start = prim.startIndex;
                int end = prim.endIndex;
                for(int f = start; f < end; f++){
                    vec4 t_quad = quadIntersect(inv_ray, faces[f]);
                    if((t_quad.x > 0) && (t_quad.x > EPSILON) && (t_quad.x < ray.t)){
                        ray.t = t_quad.x;
                        norm.x = t_quad.y;
                        norm.y = t_quad.z;
                    }
                }
            }


            break;
        }


        default:{
            break;
        }
    }
    return hit;
    /*
        default:
        {
                    if ((tQuad.x > 0) && (tQuad.x > EPSILON) && (tQuad.x < ray.t)) {
                        id = iSectID(TYPE_MESH, f, i, offset);
                        ray.t = tQuad.x;
                        norm.x = tQuad.y;
                        norm.y = tQuad.z;
                    }
                }
            }
            break;
        }
    }*/
}

float calc_shadow_ray(inout Ray ray) {
    int stack[16];
    int sp = 0; //stack pointer
    stack[0] = 0;
    vec3 invDir = 1 / ray.d;

    while (sp > -1) {
        int offset = stack[sp];
        BVHNode node = bvhNodes[offset];
        sp--;
        //int numChildren = node.numChildren & BIT_000_MAX;
        //int axis = node.numChildren >> 29;

        //if its a leaf do the regular intersection
        if (node.numChildren == 0) {
            int i = node.offset;
            switch (primitives[i].id)
            {
                case -1:
                {
                    float tSphere = sphereIntersect(ray, primitives[i]);
                    if ((tSphere > EPSILON) && (tSphere < ray.t))
                    {
                        return SHADOW;
                        ray.t = tSphere;
                    }
                    break;
                }
                case -2:
                {
                    vec4 tBox = boxIntersect(ray, primitives[i]);
                    if (tBox.x > 0) {
                        if ((tBox.x > EPSILON) && (tBox.x < ray.t)) {
                            return SHADOW;
                            ray.t = tBox.x;
                        }
                    }
                    break;
                }
                case -3:
                {
                    vec4 tCylinder = cylinderIntersect(ray, primitives[i]);
                    if ((tCylinder.x > EPSILON) && (tCylinder.x < ray.t)) {
                        return SHADOW;
                        ray.t = tCylinder.x;
                    }
                    break;
                }
                case -4:
                {
                    float tplane = planeIntersect(ray, primitives[i]);
                    if ((tplane > EPSILON) && (tplane < ray.t))
                    {
                        return SHADOW;
                        ray.t = tplane;
                    }
                    break;
                }
                case -5:
                {
                    float tDisk = diskIntersect(ray, primitives[i]);
                    if ((tDisk > EPSILON) && (tDisk < ray.t)) {
                        return SHADOW;
                        ray.t = tDisk;
                    }
                    break;
                }
                case -6:
                {
                    float tQuadTex = quadTexIntersectS(ray, primitives[i]).x;
                    if (tQuadTex > EPSILON) {
                        ray.t = tQuadTex;
                        return SHADOW;
                    }
                }
                default:
                {
                    mat4 invWorld = inverse(primitives[i].world);
                    Ray r;
                    r.d = (invWorld * vec4(ray.d, 0.0)).xyz; // / primitives[i].extents;
                    r.o = (invWorld * vec4(ray.o, 1.0)).xyz; // / primitives[i].extents;
                    flool tMesh = boundsIntersect(r); // , vec3(1, 1, 1));// primitives[i].extents);
                    if (tMesh.b && (tMesh.t > EPSILON) && (tMesh.t < ray.t)) { //hits the boundingbox, doesnt necessarily mean tri hit
                        //Mesh m = meshes[primitives[i].id];
                        //id.pId = i;
                        //rdd /= primitives[i].extents;
                        //roo /= primitives[i].extents;
                        int startIndex = primitives[i].startIndex;
                        int endIndex = primitives[i].endIndex;
                        for (int f = startIndex; f < endIndex; f++) {
                            vec4 tQuad = quadIntersect(r, faces[f]);
                            if ((tQuad.x > 0) && (tQuad.x > EPSILON) && (tQuad.x < ray.t)) {
                                return SHADOW;
                                ray.t = tQuad.x;
                            }
                        }
                    }
                    break;
                }
            }
        }
        //It's a node
        else {
            offset = offset + 1;
            if (mbvhIntersect(ray, invDir, bvhNodes[offset])) {
                sp++;
                stack[sp] = offset;
            }
            if (node.offset < bvhNodes.length()) {
                if (mbvhIntersect(ray, invDir, bvhNodes[node.offset])) {
                    sp++;
                    stack[sp] = node.offset;
                }
            }
        }
    }
    return 1.f;
}


#endif
