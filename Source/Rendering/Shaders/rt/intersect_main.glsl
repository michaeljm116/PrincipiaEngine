#ifndef _INTERSECT_MAIN_GLSL
#define _INTERSECT_MAIN_GLSL

#include "intersect_plane.glsl"
#include "intersect_box.glsl"
#include "intersect_sphere.glsl"
#include "intersect_mesh.glsl"
#include "intersect_cylinder.glsl"
#include "intersect_disk.glsl"
#include "intersect_bvh.glsl"

#define TRIINTERSECT false
#define SHADOW 0.2
//#define DEBUGLINES

const uint TYPE_SPHERE = 0x1u;
const uint TYPE_BOX = 0x2u;
const uint TYPE_PLANE = 0x4u;
const uint TYPE_MESH = 0x8u;
const uint TYPE_CYLINDER = 0x10u;
const uint TYPE_DISK = 0x20u;
const uint TYPE_BVHNODE = 0x80u;
const uint TYPE_QUAD = 0x100u;
const int BIT_000_MAX = 268435455;
const int SAMPLES = 1;
const vec2[] SampleTable = { vec2(-0.5, 0.5f), vec2(0.5f, -0.5f), vec2(0.5f, 0.5f), vec2(-0.5f, -0.5f) };

void insertSectID(iSectID[3] ids, iSectID id) {
    // If the first one is empty, place something inside immediately and end
    if (ids[0].primType == 0) {
        ids[0] = id;
    }
    // Else, compare the values, the smallest goes to the highest etc..
}

iSectID[2] intersectMBVH(inout Ray ray, inout vec3 norm, inout vec2 uv) {
    iSectID id = iSectID(0, -1, -1, -1);
    iSectID id2 = iSectID(0, -1, -1, -1);
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
        #ifdef DEBUGLINES
        if (bvhBoundsIntersect(ray, invDir, bvhNodes[offset])) {
            return iSectID(0, 0, 0, offset);
        }
        #endif
        //if its a leaf do the regular intersection
        if (node.numChildren > 0) {
            for (int i = node.offset; i < node.offset + node.numChildren; ++i) {
                switch(primitives[i].id)
                {
                    case -1:
                    {
                        float tSphere = sphereIntersect(ray, primitives[i]);
                        if ((tSphere > EPSILON) && (tSphere < ray.t)){
                            id = iSectID(TYPE_SPHERE, i, -1, offset);
                            ray.t = tSphere;
                        }
                        break;
                    }
                    case -2:{
                        vec4 tBox = boxIntersect(ray, primitives[i]);
                        if (tBox.x > 0) {
                            if ((tBox.x > EPSILON) && (tBox.x < ray.t)) {
                                id = iSectID(TYPE_BOX, i, -1, offset);
                                ray.t = tBox.x;
                                norm = tBox.yzw;
                            }
                        }
                        break;
                    }
                    case -3:{
                        vec4 tCylinder = cylinderIntersect(ray, primitives[i]);
                        if ((tCylinder.x > EPSILON) && (tCylinder.x < ray.t)) {
                            id = iSectID(TYPE_CYLINDER, i, -1, offset);
                            ray.t = tCylinder.x;
                            norm = tCylinder.yzw;
                        }
                        break;
                    }
                    case -4:{
                        float tplane = planeIntersect(ray, primitives[i]);
                        if ((tplane > EPSILON) && (tplane < ray.t))
                        {
                            id = iSectID(TYPE_PLANE, i, -1, offset);
                            ray.t = tplane;
                        }
                        break;
                    }
                    case -5:{
                        float tDisk = diskIntersect(ray, primitives[i]);
                        if ((tDisk > EPSILON) && (tDisk < ray.t)) {
                            id = iSectID(TYPE_DISK, i, -1, offset);
                            ray.t = tDisk;
                        }
                        break;
                    }
                    case -6:{
                        vec4 tQuadTex = quadTexIntersect(ray, primitives[i], uv);
                        if (tQuadTex.x > 0) {
                            if ((tQuadTex.x > EPSILON) && (tQuadTex.x < ray.t)) {
                                id2 = id;
                                ray.t2 = ray.t;
                                id = iSectID(TYPE_QUAD, i, -1, offset);
                                ray.t = tQuadTex.x;
                                norm = tQuadTex.yzw;
                            }
                            else if((tQuadTex.x > EPSILON) && (tQuadTex.x < ray.t2)){
                                id2 = iSectID(TYPE_QUAD, i, -1, offset);
                                ray.t2 = tQuadTex.x;
                                //norm = tQuadTex.yzw;
                            }
                        }
                        break;
                    }
                    default:{
                        mat4 invWorld = inverse(primitives[i].world);
                        Ray r;
                        r.d = (invWorld * vec4(ray.d, 0.0)).xyz; // / primitives[i].extents;
                        r.o = (invWorld * vec4(ray.o, 1.0)).xyz; // / primitives[i].extents;
                        flool tMesh = boundsIntersect(r); // , vec3(1, 1, 1));// primitives[i].extents);
                        if (tMesh.b && (tMesh.t > EPSILON) && (tMesh.t < ray.t)) { //hits the boundingbox, doesnt necessarily mean tri hit
                            int startIndex = primitives[i].startIndex;
                            int endIndex = primitives[i].endIndex;
                            for (int f = startIndex; f < endIndex; f++) {
                                vec4 tQuad = quadIntersect(r, faces[f]);
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
    iSectID[2] ret;
    ret[0] = id;
    ret[1] = id2;
    return ret;//{id,id2};
}

//rayO = pos, rayD = light, primitiveId = duh, t = t = length(lights[0].pos - pos);
float calcShadowBVH(inout Ray ray) {
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
        if (node.numChildren > 0) {
            for (int i = node.offset; i < node.offset + node.numChildren; ++i) {
                switch(primitives[i].id){
                    case -1:{
                        float tSphere = sphereIntersect(ray, primitives[i]);
                        if ((tSphere > EPSILON) && (tSphere < ray.t))
                        {
                            return SHADOW;
                            ray.t = tSphere;
                        }
                        break;
                    }
                    case -2:{
                        vec4 tBox = boxIntersect(ray, primitives[i]);
                        if (tBox.x > 0) {
                            if ((tBox.x > EPSILON) && (tBox.x < ray.t)) {
                                return SHADOW;
                                ray.t = tBox.x;
                            }
                        }
                        break;
                    }
                    case -3:{
                        vec4 tCylinder = cylinderIntersect(ray, primitives[i]);
                        if ((tCylinder.x > EPSILON) && (tCylinder.x < ray.t)) {
                            return SHADOW;
                            ray.t = tCylinder.x;
                        }
                        break;
                    }
                    case -4:{
                        float tplane = planeIntersect(ray, primitives[i]);
                        if ((tplane > EPSILON) && (tplane < ray.t))
                        {
                            return SHADOW;
                            ray.t = tplane;
                        }
                        break;
                    }
                    case -5:{
                        float tDisk = diskIntersect(ray, primitives[i]);
                        if ((tDisk > EPSILON) && (tDisk < ray.t)) {
                            return SHADOW;
                            ray.t = tDisk;
                        }
                        break;
                    }
                    case -6:{
                        float tQuadTex = quadTexIntersectS(ray, primitives[i]).x;
                        if (tQuadTex > EPSILON) {
                            ray.t = tQuadTex;
                            return SHADOW;
                        }
                    }
                    default:{
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
