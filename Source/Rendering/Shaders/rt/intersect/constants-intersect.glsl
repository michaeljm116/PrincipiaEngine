#ifndef CONSTANTS_INTERSECT_GLSL
#define CONSTANTS_INTERSECT_GLSL

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
const uint MAX_INTERSECTIONS = 4;

const vec2[] SampleTable = {
    vec2(-0.5, 0.5f),
    vec2(0.5f, -0.5f),
    vec2(0.5f, 0.5f),
    vec2(-0.5f, -0.5f)
};

#endif
