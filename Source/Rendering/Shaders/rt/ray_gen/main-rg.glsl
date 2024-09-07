// Ray Generation Main
// Mike Murrell (c) 8/27/2024
#ifndef MAIN_RAYGEN_GLSL
#define MAIN_RAYGEN_GLSL

#include "../structs.glsl"
#include "../layouts.glsl"

Ray generate_ray(in vec2 uv)
{
    vec3 ps;
    Ray ray;
    ps.x = (2 * uv.x - 1) * ubo.aspectRatio * ubo.fov;
    ps.y = (2 * uv.y - 1) * ubo.fov;
    ps.z = -1;

    ps = ps * mat3(ubo.rotM);

    ray.d = normalize(ps.xzy);
    ray.o = ubo.rotM[3].xyz; // * inverse(ubo.rotM)
    return ray;
}

vec4 check_gui(vec2 uv, in vec4 txt)
{
    for (int i = 0; i < guis.length(); ++i) {
        if(guis[i].alpha <= 0.f) continue;

        vec2 diff = uv - guis[i].min;
        if(diff.x < 0.f || diff.y < 0.f) continue;
        if(diff.x > guis[i].extents.x || diff.y > guis[i].extents.y) continue;

        vec2 guv = diff / guis[i].extents;
        guv.y = -guv.y;
        vec2 fin = guis[i].alignMin + guv * guis[i].alignExt;
        vec4 temp_txtr = texture(bindless_textures[nonuniformEXT(guis[i].id)], fin);
        txt += temp_txtr * temp_txtr.a;
    }
    return txt;
}

Ray[SAMPLES] main_rg(inout vec4 txtr, inout bool gui_hit)
{
    Ray rays[SAMPLES];
    for (int samp = 0; samp < SAMPLES; ++samp) {
        ivec2 dim = imageSize(resultImage);
        vec2 uv = vec2(0);
        if (SAMPLES == 1)
            uv = vec2(gl_GlobalInvocationID.xy) / dim;
        else
            uv = vec2(gl_GlobalInvocationID.xy + SampleTable[samp]) / dim;

        // Check if it hits the GUI
        txtr = check_gui(uv, txtr);
        if (txtr.a > 0.99f)
        {
            imageStore(resultImage, ivec2(gl_GlobalInvocationID.xy), txtr);
            gui_hit = true;
            return rays;
        }
        else
        {
            gui_hit = false;
            rays[samp] = generate_ray(uv);
            return rays;
        }
    }
    return rays;
}

#endif
