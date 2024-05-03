#version 450

layout(location = 0) in vec3 inViewPosition;
layout(location = 1) in vec3 inViewNormal;
layout(location = 2) in vec3 inOrgPosition;

layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform UniformBufferObject
{
	vec4 color;
	vec3 gridScale;
	bool gridded;
} Uniform;

void main()
{
	vec4 color = Uniform.color;
	
	vec3 lightViewPos = vec3(0, 0, 0);
	vec3 negLightDir = normalize(lightViewPos - inViewPosition);
	float factor = dot(negLightDir, normalize(inViewNormal));
	factor = abs(factor);

	if (factor < 0.)
	{
		color = vec4(0., 0.2, 0.1, 1.);
	}

	if (Uniform.gridded) {
		vec3 scaledOrgPos = vec3(inOrgPosition.x * Uniform.gridScale.x, inOrgPosition.y * Uniform.gridScale.y, inOrgPosition.z * Uniform.gridScale.z);

		if (scaledOrgPos.x < 0) --scaledOrgPos.x;
		if (scaledOrgPos.z < 0) --scaledOrgPos.z;

		if ((int(scaledOrgPos.x) % 2 == 0) != (int(scaledOrgPos.z) % 2 == 0)) {
			color = vec4(0.1, 0.1, 0.1, 0.);
		}

		outColor = factor * color;
	}
	else {
		outColor = vec4(factor * color.xyz, color.w);
	}
}