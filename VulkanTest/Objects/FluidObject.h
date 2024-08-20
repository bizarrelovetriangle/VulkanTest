#pragma once
#include "Interfaces/Object.h"
#include "../Renderers/SimpleVertexedRenderer.h"
#include "../CAD/GeometryCreator.h"

#include "../VulkanContext.h"
#include "../Vulkan/CommandBuffer.h"
#include "../Vulkan/Pipeline.h"
#include "../Vulkan/DescriptorSets.h"
#include "../Vulkan/Data/BufferData.h"
#include "../Renderers/FluidRenderer.h"

struct FluidUniform
{
	alignas(4) int particlesCount = 0;
};

struct Particle
{
	alignas(16) Vector3f pos;
	alignas(4) bool flag = false;
};

class FluidObject : public Object
{
public:
	FluidObject(VulkanContext& vulkanContext) : vulkanContext(vulkanContext)
	{
		float breadth = 2.;
		int dimention = 10;
		for (int i = 0; i < dimention; ++i)
		{
			for (int j = 0; j < dimention; ++j)
			{
				for (int k = 0; k < dimention; ++k)
				{
					auto getPos = [&](int v) { return (breadth * float(v) / (dimention - 1)) - breadth / 2; };
					Particle particle(Vector3f(getPos(i), getPos(j), getPos(k)), false);
					particles.push_back(particle);
				}
			}
		}

		fluidUniform.particlesCount = particles.size();

		particlesUniformBuffer = BufferData::Create(
			vulkanContext, particles, MemoryType::Universal, vk::BufferUsageFlagBits::eStorageBuffer);
		fluidUniformBuffer = BufferData::Create(
			vulkanContext, fluidUniform, MemoryType::Universal, vk::BufferUsageFlagBits::eUniformBuffer);

		auto ico = GeometryCreator::CreateIcosphere(0.2, 1);
		auto fluidRenderer = std::make_unique<FluidRenderer>(vulkanContext);
		fluidRenderer->UpdateVertexBuffer(*ico);
		fluidRenderer->descriptorSets->UpdateStorageDescriptor(*particlesUniformBuffer, 3);
		fluidRenderer->descriptorSets->UpdateUniformDescriptor(*fluidUniformBuffer, 4);

		renderer = std::move(fluidRenderer);
	}

	virtual void Render(RenderVisitor& renderVisitor) override
	{
		auto fluidRenderer = (FluidRenderer*)renderer.get();

		fluidRenderer->transformUniform.modelToWorld = ComposeMatrix();
		fluidRenderer->UpdateTransformUniformBuffer();

		auto& pipeline = *fluidRenderer->shared->pipeline;
		vulkanContext.commandBuffer->commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.graphicsPipeline);

		vulkanContext.commandBuffer->commandBuffer.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics, pipeline.pipelineLayout, 0,
			fluidRenderer->descriptorSets->descriptorSets[renderVisitor.imageIndex], {});

		vk::Buffer vertexBuffers[] = { fluidRenderer->vertexBuffer->buffer };
		vk::DeviceSize vertexOffsets[] = { 0 };
		vulkanContext.commandBuffer->commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, vertexOffsets);

		vulkanContext.commandBuffer->commandBuffer.draw(fluidRenderer->vertexBuffer->count, fluidUniform.particlesCount, 0, 0);
	}

	virtual void Dispose() override
	{
		fluidUniformBuffer->Dispose();
		particlesUniformBuffer->Dispose();
		Object::Dispose();
	}

	VulkanContext& vulkanContext;

	std::vector<Particle> particles;
	FluidUniform fluidUniform;

	std::unique_ptr<BufferData> fluidUniformBuffer;
	std::unique_ptr<BufferData> particlesUniformBuffer;
};