#version 450

layout(location = 0) in float fragFactor;
layout(location = 1) in vec2 fragTexturePos;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform Uniform
{
	vec4 baseColor;
	bool hasTexture;
	bool hasColors;
};
layout(binding = 2) uniform sampler2D texSampler;


void main()
{
	vec4 color = texture(texSampler, fragTexturePos);

	float factor = abs(fragFactor);

	if (fragFactor < 0.)
	{
		color = vec4(0., 0.2, 0.1, 1.);
	}

	outColor = factor * color;
}