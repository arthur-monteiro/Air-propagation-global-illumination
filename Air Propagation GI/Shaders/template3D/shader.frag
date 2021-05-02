#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(binding = 0) uniform sampler textureSampler;
layout(binding = 1) uniform texture2D albedo;

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

void main() 
{
	outColor = texture(sampler2D(albedo, textureSampler), inTexCoord);
}
