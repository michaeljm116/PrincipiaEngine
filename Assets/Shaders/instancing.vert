#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform UBO{
	mat4 view;
	mat4 proj;
}camera;

layout (location = 0) in vec3 pos;
layout (location = 1) in float u;
layout (location = 2) in vec3 norm;
layout (location = 3) in float v;

layout (location = 4) in vec4 world;
layout (location = 5) in vec3 extents;
layout (location = 6) in int numChildren;
layout (location = 7) in int id;
layout (location = 8) in int matId;
layout (location = 9) in int startIndex;
layout (location = 10) in int endIndex;



out gl_PerVertex
{
	vec4 gl_Position;
};

void main(){
	gl_Position = camera.proj * camera.view * world * vec4(pos, 1.0);
}