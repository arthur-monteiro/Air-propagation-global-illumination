#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 1, r8) uniform image3D voxelTexture;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec4 outColor;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main() 
{
	vec3 pos = vec3(inPosition.x, inPosition.y, inPosition.z * 2.0 - 1.0)* 0.5 + vec3(0.5);
	ivec3 pixelPos = ivec3(pos * imageSize(voxelTexture));
	imageStore(voxelTexture, ivec3(pixelPos), vec4(rand(vec2(pixelPos.x, pixelPos.y + pixelPos.z)).xxx, 1.0));

	outColor = vec4(imageLoad(voxelTexture, ivec3(pixelPos)).r, inPosition.z * 0.5 + 0.5, 0.0, 1.0);
}
