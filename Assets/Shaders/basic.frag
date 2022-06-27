#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inTangent;
layout (location = 2) in vec2 inUV;

layout(binding = 1) uniform sampler2D diffuseMap;
layout(binding = 2) uniform sampler2D specularMap;
layout(binding = 3) uniform sampler2D normalMap;


void main(){
	vec4 diffColor = texture(diffuseMap, inUV);
	outColor = diffColor + texture(specularMap, inUV);
	//outColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
