#ifndef _LAYOUTS_GLSL
#define _LAYOUTS_GLSL
#extension GL_EXT_nonuniform_qualifier : enable

layout(std140, binding = 1) uniform UBO
{
    mat4 rotM;
    float fov;
    float aspectRatio;
    int rand;
} ubo;

layout(std430, binding = 2) buffer Verts {
    Vert verts[];
};

layout(std430, binding = 3) buffer Faces {
    Face faces[];
};

layout(std430, binding = 4) buffer BlasNodes {
    BVHNode blas[];
};

layout(std430, binding = 5) buffer Shapes {
    Shape shapes[];
};

layout(std430, binding = 6) buffer Primitives {
    Primitive primitives[];
};

layout(std430, binding = 7) buffer Materials {
    Material materials[];
};

layout(std430, binding = 8) buffer Lights {
    Light lights[];
};

layout(std430, binding = 9) buffer Guis {
    Gui guis[];
};

layout(std430, binding = 10) buffer BvhNodes {
    BVHNode bvhNodes[];
};

layout(binding = 11) uniform sampler2D guiTest[5];

layout(binding = 12) uniform sampler2D bindless_textures[];

#endif
