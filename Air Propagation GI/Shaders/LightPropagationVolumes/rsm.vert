#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
	mat4 projectionLight;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in uint inMaterialID;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out uint outMaterialID;
layout(location = 3) out vec3 outNormal;

out gl_PerVertex
{
    vec4 gl_Position;
};

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() 
{
	gl_Position = ubo.projectionLight * vec4(inPosition, 1.0);

	outPosition = inPosition;
	outTexCoord = inTexCoord;
	outMaterialID = inMaterialID;
	outNormal = inNormal;
} 
