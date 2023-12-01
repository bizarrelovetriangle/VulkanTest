#version 450

layout(location = 0) in vec3 outPosition;
layout(location = 1) in float outFactor;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform UniformBufferObject
{
	vec4 color;
	bool grided;
} Uniform;

void main()
{
	vec4 color = Uniform.color;
	float factor = abs(outFactor);

	if (outFactor < 0.)
	{
		color = vec4(0., 0.2, 0.1, 1.);
	}

	outColor = factor * color;
}