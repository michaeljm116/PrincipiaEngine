#ifndef _LAYOUTS_GLSL
#define _LAYOUTS_GLSL

layout(binding = 1) uniform UBO
{
	vec3 pos;
	float aspectRatio;
	vec3 lookat;
	float fov;
} ubo;


layout(std140, binding = 2) buffer Verts
{
	Vert verts[];
};

layout(std140, binding = 3) buffer Faces {
	Face faces[];
};

layout(std140, binding = 4) buffer Primitives{
	Primitive primitives[];
};

layout(std140, binding = 5) buffer Materials {
	Material materials[];
};

layout(std140, binding = 6) buffer Lights {
	Light lights[];
};

layout(std140, binding = 7) buffer Guis {
	Gui guis[];
};

layout(binding = 8) uniform sampler2D guiTest[5];

#endif