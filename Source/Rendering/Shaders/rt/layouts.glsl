#ifndef _LAYOUTS_GLSL
#define _LAYOUTS_GLSL

layout(binding = 1) uniform UBO
{
	mat4 rotM;
	float fov;
	float aspectRatio;
		
} ubo;


layout(std140, binding = 2) buffer Verts
{
	Vert verts[];
};

layout(std140, binding = 3) buffer Faces {
	Face faces[];
};

layout(std140, binding = 4) buffer BlasNodes {
	BVHNode blas[];
};

layout(std140, binding = 5) buffer Shapes {
	Shape shapes[];
};

layout(std140, binding = 6) buffer Primitives{
	Primitive primitives[];
};

layout(std140, binding = 7) buffer Materials {
	Material materials[];
};

layout(std140, binding = 8) buffer Lights {
	Light lights[];
};

layout(std140, binding = 9) buffer Guis {
	Gui guis[];
};

layout(std140, binding = 10) buffer BvhNodes {
	BVHNode bvhNodes[];
};

layout(binding = 11) uniform sampler2D guiTest[5];

#endif