#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform PerFrameUniform{
	mat4 view;
	mat4 proj;
	mat4 model;
}pfu;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inUV;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outTangent;
layout (location = 2) out vec2 outUV;

out gl_PerVertex 
{
	vec4 gl_Position;
};

void main(){
		
	gl_Position = pfu.proj * pfu.view * pfu.model * vec4(inPosition, 1.0);

	
	//outWorldPos = vec3(pfu.model * vec4(inPosition, 1.0));
	outNormal = inNormal;
	outTangent = inTangent;
	outUV = inUV;

}

