#version 450

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
	outColor = color;
}