#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4 projectionX;
	mat4 projectionY;
	mat4 projectionZ;

	mat4[4] lightSpaceMatrices;
    vec4 cascadeSplits;
	mat4 modelView;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in uint inMaterialID;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outRawPosition;
layout(location = 2) out vec3 outViewPos;
layout(location = 3) out vec2 outTexCoord;
layout(location = 4) out uint outMaterialID;
layout(location = 5) out vec3 outNormal;
layout(location = 6) out vec4 outCascadeSplits;
layout(location = 7) out vec4 outPosLightSpace[4];

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
	// Find which projection is the best
	float projX = abs(dot(inNormal, vec3(1.0, 0.0, 0.0)));
	float projY = abs(dot(inNormal, vec3(0.0, 1.0, 0.0)));
	float projZ = abs(dot(inNormal, vec3(0.0, 0.0, 1.0)));
	
	if(projX > projY && projX > projZ)
	{
		gl_Position = ubo.projectionX * vec4(inPosition, 1.0);
		outPosition = gl_Position.zyx;
		outPosition.x = (1.0 - outPosition.x) * 2.0 - 1.0; // [-1.0; 1.0]
		outPosition.z = outPosition.z * 0.5 + 0.5; // [0; 1]
	}
	else if(projY > projX && projY > projZ)
	{
		gl_Position = ubo.projectionY * vec4(inPosition, 1.0);
		outPosition = gl_Position.xzy;
		outPosition.x = -outPosition.x;
		outPosition.y = outPosition.y * 2.0 - 1.0; // [-1.0; 1.0]
		outPosition.z = outPosition.z * 0.5 + 0.5; // [0; 1]
	}
	else
	{
		gl_Position = ubo.projectionZ * vec4(inPosition, 1.0);
		outPosition = gl_Position.xyz;
	}

	outTexCoord = inTexCoord;
	outMaterialID = inMaterialID;
	outNormal = inNormal;
	outRawPosition = inPosition;
	outCascadeSplits = ubo.cascadeSplits;
	outViewPos = (ubo.modelView * vec4(inPosition, 1.0)).xyz;
	
	for(int i = 0; i < 4; i++)
        outPosLightSpace[i] = biasMat * ubo.lightSpaceMatrices[i] * vec4(inPosition, 1.0);
} 
