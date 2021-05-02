#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(binding = 1) uniform sampler textureSampler;
layout(binding = 2) uniform texture2D[] textures;

layout(location = 0) in vec3 inViewPos;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) flat in uint inMaterialID;
layout(location = 3) in mat3 inTBN;

layout(location = 0) out vec4 outNormalRoughnessMetal;
layout(location = 1) out vec4 outAlbedoAlpha;

vec3 sRGBToLinear(vec3 color)
{
	if (length(color) <= 0.04045)
    	return color / 12.92;
	else
		return pow((color + vec3(0.055)) / 1.055, vec3(2.4));
}

vec4 encode (vec3 n, vec3 view)
{
    float p = sqrt(n.z * 8.0 + 8.0);
    return vec4(n.xy / p + vec2(0.5), 0.0, 0.0);
}

void main() 
{
	vec3 normal = (texture(sampler2D(textures[inMaterialID * 5 + 1], textureSampler), inTexCoord).rgb * 2.0 - vec3(1.0)) * inTBN;
	normal = normalize(normal);

	outNormalRoughnessMetal = vec4(encode(normal, inViewPos).xy, texture(sampler2D(textures[inMaterialID * 5 + 2], textureSampler), inTexCoord).r, 
		texture(sampler2D(textures[inMaterialID * 5 + 3], textureSampler), inTexCoord).r);

	outAlbedoAlpha = vec4(sRGBToLinear(texture(sampler2D(textures[inMaterialID * 5], textureSampler), inTexCoord).rgb), 1.0);
}
