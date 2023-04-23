#version 450

layout(location = 0) in float fragFactor;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in vec2 fragTexturePos;
layout(binding = 0) uniform Uniform
{
	vec4 baseColor;
	bool hasTexture;
	bool hasColors;
};
layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main()
{
	vec4 color = baseColor;
	if (hasTexture)		color = texture(texSampler, fragTexturePos);
	else if (hasColors)	color = fragColor;

	float factor = abs(fragFactor);

	if (fragFactor < 0.)
	{
		color = vec4(0., 0.2, 0.1, 1.);
	}

	outColor = factor * color;
}