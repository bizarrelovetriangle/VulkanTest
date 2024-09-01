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

	alignas(4) float gridCellSize = 0;
	alignas(16) Vector3i gridDimention = {};
	alignas(16) Vector3f gridSize = {};
};

struct Particle
{
	alignas(16) Vector3f pos;
	alignas(4) bool valid = false;
	alignas(4) int gridCellIndex = 0;
};

struct GridCell
{
	alignas(4) int count = 0;
	alignas(4) int offset = 0;
};

class ComputeProgram
{
public:
	ComputeProgram(VulkanContext& vulkanContext, const std::string& shaderPath, const std::string& entryPoint,
		FluidUniform& fluidUniform,
		std::unique_ptr<BufferData>& fluidUniformBuffer, std::unique_ptr<BufferData>& particlesStorageBuffer,
		std::unique_ptr<BufferData>& particlesStorageBufferCopy, std::unique_ptr<BufferData>& gridStorageBuffer)
		: vulkanContext(vulkanContext),
		fluidUniform(fluidUniform), fluidUniformBuffer(fluidUniformBuffer), particlesStorageBuffer(particlesStorageBuffer),
		particlesStorageBufferCopy(particlesStorageBufferCopy), gridStorageBuffer(gridStorageBuffer)
	{
		auto& device = vulkanContext.deviceController->device;

		auto computeSpirv = ShaderCompiler::CompileShader(shaderPath, vk::ShaderStageFlagBits::eCompute, false, entryPoint);
		computeShaderModule = device.createShaderModule(vk::ShaderModuleCreateInfo({}, computeSpirv));
		vk::PipelineShaderStageCreateInfo vertShaderStageInfo(
			{}, vk::ShaderStageFlagBits::eCompute, computeShaderModule, "main");

		auto descriptorBindings = std::vector{
			vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAll),
			vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAll),
			vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAll),
			vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAll),
		};

		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreate({}, descriptorBindings);
		computeDescriptorSetLayout = device.createDescriptorSetLayout(descriptorSetLayoutCreate);

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, computeDescriptorSetLayout, {});
		computePipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);

		vk::ComputePipelineCreateInfo pipeliceCreateInfo({}, { vertShaderStageInfo }, computePipelineLayout);

		computePipeline = device.createComputePipeline(nullptr, pipeliceCreateInfo).value;

		computeDescriptorSet = std::make_unique<DescriptorSets>(vulkanContext, computeDescriptorSetLayout, descriptorBindings);
		computeDescriptorSet->UpdateStorageDescriptor(*fluidUniformBuffer, 0);
		computeDescriptorSet->UpdateStorageDescriptor(*particlesStorageBuffer, 1);
		computeDescriptorSet->UpdateStorageDescriptor(*particlesStorageBufferCopy, 2);
		computeDescriptorSet->UpdateStorageDescriptor(*gridStorageBuffer, 3);
	}

	void Run(vk::CommandBuffer& cb, int imageIndex, int groupCountX)
	{
		cb.bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline);
		cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, computePipelineLayout, 0, computeDescriptorSet->descriptorSets[imageIndex], {});
		cb.dispatch(groupCountX, 1, 1);
	}

	void Dispose()
	{
		auto& device = vulkanContext.deviceController->device;
		device.destroyPipeline(computePipeline);
		device.destroyPipelineLayout(computePipelineLayout);
		device.destroyDescriptorSetLayout(computeDescriptorSetLayout);
		device.destroyShaderModule(computeShaderModule);
		computeDescriptorSet->Dispose();
	}

	VulkanContext& vulkanContext;

	vk::Pipeline computePipeline;
	vk::ShaderModule computeShaderModule;
	vk::PipelineLayout computePipelineLayout;
	std::unique_ptr<DescriptorSets> computeDescriptorSet;
	vk::DescriptorSetLayout computeDescriptorSetLayout;

	FluidUniform& fluidUniform;
	std::unique_ptr<BufferData>& fluidUniformBuffer;
	std::unique_ptr<BufferData>& particlesStorageBuffer;
	std::unique_ptr<BufferData>& particlesStorageBufferCopy;
	std::unique_ptr<BufferData>& gridStorageBuffer;
};

class FluidObject : public Object
{
public:
	FluidObject(VulkanContext& vulkanContext) : vulkanContext(vulkanContext)
	{
		float breadth = 2.;
		int dimention = 2;
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
		fluidUniform.gridCellSize = 0.5;
		fluidUniform.gridDimention = { 100, 100, 100 };

		fluidUniform.gridSize = fluidUniform.gridDimention * fluidUniform.gridCellSize;

		auto grid = std::vector<GridCell>(fluidUniform.gridDimention.x * fluidUniform.gridDimention.y * fluidUniform.gridDimention.z);

		fluidUniformBuffer = BufferData::Create(
			vulkanContext, fluidUniform, MemoryType::Universal, vk::BufferUsageFlagBits::eStorageBuffer);
		particlesStorageBuffer = BufferData::Create(
			vulkanContext, particles, MemoryType::Universal, vk::BufferUsageFlagBits::eStorageBuffer);
		particlesStorageBufferCopy = BufferData::Create(
			vulkanContext, particles, MemoryType::Universal, vk::BufferUsageFlagBits::eStorageBuffer);
		gridStorageBuffer = BufferData::Create(
			vulkanContext, grid, MemoryType::DeviceLocal, vk::BufferUsageFlagBits::eStorageBuffer);

		auto icosphere = GeometryCreator::CreateIcosphere(0.2, 1);
		auto fluidRenderer = std::make_unique<FluidRenderer>(vulkanContext);
		fluidRenderer->UpdateVertexBuffer(*icosphere);
		fluidRenderer->descriptorSets->UpdateStorageDescriptor(*particlesStorageBuffer, 3);
		fluidRenderer->descriptorSets->UpdateStorageDescriptor(*fluidUniformBuffer, 4);

		renderer = std::move(fluidRenderer);

		determineGridCellsProgram = std::make_unique<ComputeProgram>(vulkanContext,
			"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/Compute/fluid.comp", "determineGridCells",
			fluidUniform, fluidUniformBuffer, particlesStorageBuffer, particlesStorageBufferCopy, gridStorageBuffer);
		countGridCellsOffsetProgram = std::make_unique<ComputeProgram>(vulkanContext,
			"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/Compute/fluid.comp", "countGridCellsOffset",
			fluidUniform, fluidUniformBuffer, particlesStorageBuffer, particlesStorageBufferCopy, gridStorageBuffer);
		distributeByCellsProgram = std::make_unique<ComputeProgram>(vulkanContext,
			"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/Compute/fluid.comp", "distributeByCells",
			fluidUniform, fluidUniformBuffer, particlesStorageBuffer, particlesStorageBufferCopy, gridStorageBuffer);
		moveParticlesProgram = std::make_unique<ComputeProgram>(vulkanContext,
			"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/Compute/fluid.comp", "moveParticles",
			fluidUniform, fluidUniformBuffer, particlesStorageBuffer, particlesStorageBufferCopy, gridStorageBuffer);
	}

	void Run(vk::CommandBuffer& cb, int imageIndex)
	{
		//determineGridCellsProgram->Run(cb, imageIndex, fluidUniform.particlesCount);
		//countGridCellsOffsetProgram->Run(cb, imageIndex, 1);
		//distributeByCellsProgram->Run(cb, imageIndex, fluidUniform.particlesCount);
		moveParticlesProgram->Run(cb, imageIndex, fluidUniform.particlesCount);
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
		particlesStorageBuffer->Dispose();
		particlesStorageBufferCopy->Dispose();
		gridStorageBuffer->Dispose();

		determineGridCellsProgram->Dispose();
		countGridCellsOffsetProgram->Dispose();
		distributeByCellsProgram->Dispose();
		moveParticlesProgram->Dispose();

		Object::Dispose();
	}

	VulkanContext& vulkanContext;

	std::vector<Particle> particles;
	FluidUniform fluidUniform;

	std::unique_ptr<BufferData> fluidUniformBuffer;
	std::unique_ptr<BufferData> particlesStorageBuffer;
	std::unique_ptr<BufferData> particlesStorageBufferCopy;
	std::unique_ptr<BufferData> gridStorageBuffer;

	std::unique_ptr<ComputeProgram> determineGridCellsProgram;
	std::unique_ptr<ComputeProgram> countGridCellsOffsetProgram;
	std::unique_ptr<ComputeProgram> distributeByCellsProgram;
	std::unique_ptr<ComputeProgram> moveParticlesProgram;
};