#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec4 inColor;

layout(binding = 2) uniform Uniform
{
	vec4 baseColor;
	bool hasTexture;
	bool hasColors;
};

void main()
{
	vec4 color = baseColor;
	outColor = inColor;
}