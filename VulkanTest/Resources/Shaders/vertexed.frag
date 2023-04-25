#version 450

layout(location = 0) in float fragFactor;
layout(binding = 0) uniform Uniform
{
	vec4 baseColor;
	bool hasTexture;
	bool hasColors;
};

layout(location = 0) out vec4 outColor;

void main()
{
	vec4 color = baseColor;

	float factor = abs(fragFactor);

	if (fragFactor < 0.)
	{
		color = vec4(0., 0.2, 0.1, 1.);
	}

	outColor = factor * color;
}