#version 450

layout(location = 0) in float fragFactor;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in vec2 fragTexturePos;
layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main()
{
	vec4 textureColor = texture(texSampler, fragTexturePos);
	outColor = fragFactor * fragColor;
}