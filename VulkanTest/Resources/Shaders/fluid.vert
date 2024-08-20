#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 outViewPosition;
layout(location = 1) out vec3 outViewNormal;

layout(binding = 0) uniform CommonUniform
{
	mat4x4 worldToView;
	mat4x4 viewToProj;
} Common;

layout(binding = 1) uniform TransformUniform
{
	mat4x4 modelToWorld;
} Transform;

struct Particle
{
	vec3 pos;
	bool flag;
};

layout(binding = 3) buffer ParticlesUniform
{
	Particle particles[];
} Particles;

layout(binding = 4) uniform FluidUniform
{
	int particlesCount;
} Fluid;

void main()
{
	mat4 modelToWorld = mat4(1.);
	int val = gl_InstanceIndex;
	modelToWorld[3].xyz = Particles.particles[val].pos;

	mat4 modelToView = Common.worldToView * modelToWorld;
	vec3 viewPos = vec3(modelToView * vec4(pos, 1.));
	outViewPosition = viewPos;

	mat3 normalMatrix = transpose(inverse(mat3(modelToView)));
	outViewNormal = normalize(normalMatrix * normal);

	gl_Position = Common.viewToProj * vec4(viewPos, 1.0);
}