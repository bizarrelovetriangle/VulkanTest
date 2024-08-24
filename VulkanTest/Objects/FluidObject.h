#pragma once
#include "Interfaces/Object.h"
#include "../Renderers/SimpleVertexedRenderer.h"
#include "../CAD/GeometryCreator.h"

#include "../VulkanContext.h"
#include "../Vulkan/Pipeline.h"
#include "../Vulkan/DescriptorSets.h"
#include "../Vulkan/Data/BufferData.h"
#include "../Renderers/FluidRenderer.h"
#include "../Utils/ShaderCompiler.h"

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
		int dimention = 5;
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

		auto icosphere = GeometryCreator::CreateIcosphere(0.2, 1);
		auto fluidRenderer = std::make_unique<FluidRenderer>(vulkanContext);
		fluidRenderer->UpdateVertexBuffer(*icosphere);
		fluidRenderer->descriptorSets->UpdateStorageDescriptor(*particlesUniformBuffer, 3);
		fluidRenderer->descriptorSets->UpdateUniformDescriptor(*fluidUniformBuffer, 4);

		renderer = std::move(fluidRenderer);




		auto& device = vulkanContext.deviceController->device;

		auto computeSpirv = ShaderCompiler::CompileShader("E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/Compute/fluid.comp",
			vk::ShaderStageFlagBits::eCompute, false);
		computeShaderModule = device.createShaderModule(vk::ShaderModuleCreateInfo({}, computeSpirv));

		vk::PipelineShaderStageCreateInfo vertShaderStageInfo(
			{}, vk::ShaderStageFlagBits::eCompute, computeShaderModule, "main");

		auto descriptorBindings = std::vector{
			vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAll),
			vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		};

		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreate({}, descriptorBindings);
		computeDescriptorSetLayout = device.createDescriptorSetLayout(descriptorSetLayoutCreate);

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, computeDescriptorSetLayout, {});
		computePipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);

		vk::ComputePipelineCreateInfo pipeliceCreateInfo({}, { vertShaderStageInfo }, computePipelineLayout);

		computePipeline = device.createComputePipeline(nullptr, pipeliceCreateInfo).value;

		computeDescriptorSet = std::make_unique<DescriptorSets>(vulkanContext, computeDescriptorSetLayout, descriptorBindings);
		computeDescriptorSet->UpdateStorageDescriptor(*particlesUniformBuffer, 0);
		computeDescriptorSet->UpdateUniformDescriptor(*fluidUniformBuffer, 1);
	}

	void Run(vk::CommandBuffer& cb, int imageIndex)
	{
		cb.bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline);
		cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, computePipelineLayout, 0, computeDescriptorSet->descriptorSets[imageIndex], {});
		cb.dispatch(particles.size(), 1, 1);
	}

	virtual void Render(RenderVisitor& renderVisitor) override
	{
		auto fluidRenderer = (FluidRenderer*)renderer.get();

		fluidRenderer->transformUniform.modelToWorld = ComposeMatrix();
		fluidRenderer->UpdateTransformUniformBuffer();

		auto& pipeline = *fluidRenderer->shared->pipeline;
		renderVisitor.commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.graphicsPipeline);

		renderVisitor.commandBuffer.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics, pipeline.pipelineLayout, 0,
			fluidRenderer->descriptorSets->descriptorSets[renderVisitor.imageIndex], {});

		vk::Buffer vertexBuffers[] = { fluidRenderer->vertexBuffer->buffer };
		vk::DeviceSize vertexOffsets[] = { 0 };
		renderVisitor.commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, vertexOffsets);

		renderVisitor.commandBuffer.draw(fluidRenderer->vertexBuffer->count, fluidUniform.particlesCount, 0, 0);
	}

	virtual void Dispose() override
	{
		fluidUniformBuffer->Dispose();
		particlesUniformBuffer->Dispose();

		auto& device = vulkanContext.deviceController->device;
		device.destroyPipeline(computePipeline);
		device.destroyPipelineLayout(computePipelineLayout);
		device.destroyDescriptorSetLayout(computeDescriptorSetLayout);
		device.destroyShaderModule(computeShaderModule);
		computeDescriptorSet->Dispose();

		Object::Dispose();
	}

	VulkanContext& vulkanContext;

	std::vector<Particle> particles;
	FluidUniform fluidUniform;

	std::unique_ptr<BufferData> fluidUniformBuffer;
	std::unique_ptr<BufferData> particlesUniformBuffer;

	vk::Pipeline computePipeline;
	vk::ShaderModule computeShaderModule;
	vk::PipelineLayout computePipelineLayout;
	std::unique_ptr<DescriptorSets> computeDescriptorSet;
	vk::DescriptorSetLayout computeDescriptorSetLayout;
};