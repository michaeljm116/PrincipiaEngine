#ifndef _MAIN_INTERSECT_GLSL
#define _MAIN_INTERSECT_GLSL

#include "plane.glsl"
#include "box.glsl"
#include "sphere.glsl"
#include "mesh.glsl"
#include "cylinder.glsl"
#include "disk.glsl"
#include "bvh.glsl"
#include "helpers.glsl"
#include "traverse.glsl"
#include "../structs.glsl"
#include "../layouts.glsl"

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

HitInfo intersect_closest(inout Ray ray)
{
    // Set up
    HitInfo info = HitInfo(MAXLEN, vec3(0), 0, -1, -1, -1);
    int stack[16];
    int sp = 0; //stack pointer
    stack[0] = 0;
    vec3 invDir = 1 / ray.d;

    // Main Intersection Loop
    while (sp > -1)
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
        if (node.numChildren == 0)
        {
            HitInfo temp_info = traverse(ray, node.offset, offset);
            if (temp_info.t < info.t) {
                info = temp_info;
                ray.t = info.t;
            }
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
    return info;
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
            float ret = quick_traverse(ray, node.offset, SHADOW);
            if (ret == SHADOW) return SHADOW;
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
