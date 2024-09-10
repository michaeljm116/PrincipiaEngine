#ifndef _INTERSECT_BOX_GLSL
#define _INTERSECT_BOX_GLSL
#include "../structs.glsl"
vec4 boxIntersect(in Ray ray, in Primitive box)
{
    // convert from ray to box space
    // currently 147/148
    mat4 invWorld = inverse(box.world);
    vec3 rdd = (invWorld * vec4(ray.d, 0.0)).xyz;
    vec3 roo = (invWorld * vec4(ray.o, 1.0)).xyz;

    // ray-box intersection in box space
    vec3 m = 1.0 / rdd;
    vec3 n = m * roo;
    vec3 k = abs(m) * box.extents;

    vec3 t1 = -n - k;
    vec3 t2 = -n + k;

    float tN = max(max(t1.x, t1.y), t1.z);
    float tF = min(min(t2.x, t2.y), t2.z);
    if (tN > tF || tF < 0.0) return vec4(-1.0);

    vec3 nor = -sign(rdd) * step(t1.yzx, t1.xyz) * step(t1.zxy, t1.xyz);

    // convert to ray space

    nor = (box.world * vec4(nor, 0.0)).xyz;

    return vec4(tN, nor);
}

vec4 quadTexIntersect(in Ray ray, in Primitive quad, inout vec2 uv)
{
    return vec4(1.0);
    // Convert from ray to quad space
    mat4 invWorld = inverse(quad.world);
    vec3 rdd = (invWorld * vec4(ray.d, 0.0)).xyz;
    vec3 roo = (invWorld * vec4(ray.o, 1.0)).xyz;

    // Since we're working in the XY plane, ignore the Z component
    rdd.z = 0.0;
    roo.z = 0.0;

    // Early out if the ray is parallel to the plane of the quad
    if (abs(rdd.x) < EPSILON && abs(rdd.y) < EPSILON) {
        return vec4(-1.0);
    }

    // Ray-quad intersection in quad space (2D)
    vec2 m = 1.0 / rdd.xy;
    vec2 n = m * roo.xy;
    vec2 k = abs(m) * quad.extents.xy;

    vec2 t1 = -n - k;
    vec2 t2 = -n + k;

    float tN = max(t1.x, t1.y);
    float tF = min(t2.x, t2.y);
    if (tN > tF || tF < 0.0) {
        return vec4(-1.0);
    }

    // Determine the normal based on the direction of the ray
    vec2 nor2D = -sign(rdd.xy) * step(t1.yx, t1.xy);

    // Convert to ray space
    vec3 nor = vec3(nor2D, 0.0);
    nor = (quad.world * vec4(nor, 0.0)).xyz;

    //Get Texture coordinates
    vec3 pos = rdd * tN + roo;
    vec3 cen = quad.world[3].xyz;
    vec3 ext = quad.extents.xyz;

    vec2 newPos = abs(pos.xy - cen.xy);
    newPos = newPos / ext.xy;
    uv = vec2(newPos);

    return vec4(tN, nor);
}
vec4 quadTexIntersectS(in Ray ray, in Primitive quad)
{
    return vec4(1.0);
    // Convert from ray to quad space
    mat4 invWorld = inverse(quad.world);
    vec3 rdd = (invWorld * vec4(ray.d, 0.0)).xyz;
    vec3 roo = (invWorld * vec4(ray.o, 1.0)).xyz;

    // Since we're working in the XY plane, ignore the Z component
    rdd.z = 0.0;
    roo.z = 0.0;

    // Early out if the ray is parallel to the plane of the quad
    if (abs(rdd.x) < EPSILON && abs(rdd.y) < EPSILON) {
        return vec4(-1.0);
    }

    // Ray-quad intersection in quad space (2D)
    vec2 m = 1.0 / rdd.xy;
    vec2 n = m * roo.xy;
    vec2 k = abs(m) * quad.extents.xy;

    vec2 t1 = -n - k;
    vec2 t2 = -n + k;

    float tN = max(t1.x, t1.y);
    float tF = min(t2.x, t2.y);
    if (tN > tF || tF < 0.0) {
        return vec4(-1.0);
    }

    // Determine the normal based on the direction of the ray
    vec2 nor2D = -sign(rdd.xy) * step(t1.yx, t1.xy);

    // Convert to ray space
    vec3 nor = vec3(nor2D, 0.0);
    nor = (quad.world * vec4(nor, 0.0)).xyz;

    //Get Texture coordinates
    //vec3 pos = rdd * tN + roo;
    //vec3 cen = quad.world[3].xyz;
    //vec3 ext = quad.extents.xyz;

    return vec4(tN, nor);
}
vec3 boxTexture(in vec3 pos, in vec3 norm, in Primitive box, sampler2D t) {
    mat4 invWorld = inverse(box.world);
    vec3 div = 1 / box.extents * 0.5f;
    vec3 iPos = (vec4(pos, 1) * invWorld).xyz;
    vec3 iNorm = (vec4(norm, 0) * invWorld).xyz;

    vec4 xTxtr = texture(t, div.x * iPos.yz);
    vec4 yTxtr = texture(t, div.y * iPos.zx);
    vec4 zTxtr = texture(t, div.z * iPos.xy);

    vec3 ret =
        abs(iNorm.x) * xTxtr.rgb * xTxtr.a +
            abs(iNorm.y) * yTxtr.rgb * yTxtr.a +
            abs(iNorm.z) * zTxtr.rgb * zTxtr.a;
    return ret;
}
#endif
