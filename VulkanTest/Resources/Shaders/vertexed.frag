#version 450

layout(location = 0) in vec3 inViewPosition;
layout(location = 1) in vec3 inViewNormal;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform Uniform
{
	vec4 baseColor;
	bool hasTexture;
	bool hasColors;
};

void main()
{
	vec4 color = baseColor;
	
	vec3 lightViewPos = vec3(0, 0, 0);
	vec3 negLightDir = normalize(lightViewPos - inViewPosition);
	float factor = dot(negLightDir, normalize(inViewNormal));
	factor = abs(factor);

	if (factor < 0.)
	{
		color = vec4(0., 0.2, 0.1, 1.);
	}

	outColor = factor * color;
}