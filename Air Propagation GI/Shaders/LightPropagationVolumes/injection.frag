#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(binding = 1) uniform sampler textureSampler;
layout(binding = 2, r32ui) uniform coherent uimage3D[7] voxelData;
layout(binding = 9) uniform texture2D[] textures;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inRawPosition;
layout(location = 2) in vec3 inViewPos;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) flat in uint inMaterialID;
layout(location = 5) in vec3 inNormal;
layout(location = 6) in vec4 inCascadeSplits;
layout(location = 7) in vec4 inPosLightSpace[4];

layout(location = 0) out vec4 outColor;

const int CASCADE_COUNT = 4;
const float BIAS = 0.001;

void main() 
{
	uint cascadeIndex = 0;
	for(uint i = 0; i < CASCADE_COUNT; ++i) 
	{
		if(-inViewPos.z <= inCascadeSplits[i])
		{	
			cascadeIndex = i;
			break;
		}
	}

	vec3 projCoords = inPosLightSpace[cascadeIndex].xyz / inPosLightSpace[cascadeIndex].w; 
	if(projCoords.x < 0 || projCoords.x > 1 || projCoords.y < 0 || projCoords.y > 1)
		return;
	float closestDepth = texture(sampler2D(textures[cascadeIndex], textureSampler), projCoords.xy).r; 

	if(projCoords.z - BIAS > closestDepth)
		return;

	vec3 pos = vec3(inPosition.x, inPosition.y, inPosition.z * 2.0 - 1.0)* 0.5 + vec3(0.5);
	ivec3 pixelPos = ivec3(pos * imageSize(voxelData[0]));

	vec3 color = texture(sampler2D(textures[inMaterialID * 5 + CASCADE_COUNT], textureSampler), inTexCoord).rgb;

	imageAtomicAdd(voxelData[0], ivec3(pixelPos), int(color.r * 255.));
	imageAtomicAdd(voxelData[1], ivec3(pixelPos), int(color.g * 255.));
	imageAtomicAdd(voxelData[2], ivec3(pixelPos), int(color.b * 255.));
	imageAtomicAdd(voxelData[3], ivec3(pixelPos), int(inNormal.r * 255.));
	imageAtomicAdd(voxelData[4], ivec3(pixelPos), int(inNormal.g * 255.));
	imageAtomicAdd(voxelData[5], ivec3(pixelPos), int(inNormal.b * 255.));
	imageAtomicAdd(voxelData[6], ivec3(pixelPos), 1);

	outColor = vec4(color, 1.0);
}
