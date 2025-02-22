#version 450

layout(location = 0) in vec3 inViewPosition;
layout(location = 1) in vec3 inViewNormal;
layout(location = 3) in vec3 inModelPosition;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform CommonUniform
{
	mat4x4 worldToView;
	mat4x4 viewToProj;
} Common;

layout(binding = 1) uniform TransformUniform
{
	mat4x4 modelToWorld;
} Transform;

layout(binding = 2) uniform Uniform
{
	vec4 baseColor;
	bool hasTexture;
	bool hasColors;
};

void main()
{
	float size = 1.;
	vec4 color = baseColor;

	if (inModelPosition.length() > size)
	{
		//outColor = vec4(0., 0., 0., 0.);
		//return;
	}

	vec3 newOrgPosition = size * normalize(inModelPosition);
	vec3 newViewPosition = vec3(Transform.modelToWorld * vec4(newOrgPosition, 1.));

	mat3 normalMatrix = transpose(inverse(mat3(Transform.modelToWorld)));
	vec3 normal = normalize(normalMatrix * (newOrgPosition / size));

	vec3 lightViewPos = vec3(inverse(Common.worldToView) * vec4(0, 0, 0, 1.));
	vec3 negLightDir = normalize(lightViewPos - newViewPosition);
	float factor = dot(negLightDir, normal);
	factor = abs(factor);

	if (factor < 0.)
	{
		color = vec4(0., 0.2, 0.1, 1.);
	}

	outColor = vec4(factor * color.xyz, color.w);
}