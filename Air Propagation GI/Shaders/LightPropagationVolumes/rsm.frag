#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(binding = 1) uniform sampler textureSampler;
layout(binding = 2) uniform texture2D[] textures;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) flat in uint inMaterialID;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outPosition;
layout(location = 2) out vec4 outNormal;

void main() 
{
	outColor = vec4(texture(sampler2D(textures[inMaterialID * 5], textureSampler), inTexCoord).rgb, 1.0);
	outPosition = vec4(inPosition, 1.0);
	outNormal = vec4(inNormal, 1.0);
}
