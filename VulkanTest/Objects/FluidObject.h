#pragma once
#include "Interfaces/Object.h"
#include "../Renderers/SimpleVertexedRenderer.h"
#include "../CAD/GeometryCreator.h"
#include "Primitives/BoundingBoxObject.h"

#include "../VulkanContext.h"
#include "../Vulkan/Pipeline.h"
#include "../Vulkan/DescriptorSets.h"
#include "../Vulkan/Data/BufferData.h"
#include "../Renderers/FluidRenderer.h"
#include "../Utils/ShaderCompiler.h"

#undef MemoryBarrier;

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

struct IndirectDispatch
{
	alignas(16) vk::DispatchIndirectCommand particlesCountDispatch{};
	alignas(16) vk::DrawIndirectCommand particlesCountDraw{};
};

class ComputeProgram
{
public:
	ComputeProgram(VulkanContext& vulkanContext, const std::string& shaderPath, const std::string& entryPoint,
		FluidUniform& fluidUniform, IndirectDispatch& indirectDispatch,
		std::unique_ptr<BufferData>& fluidUniformBuffer, std::unique_ptr<BufferData>& particlesStorageBuffer,
		std::unique_ptr<BufferData>& particlesStorageBufferCopy, std::unique_ptr<BufferData>& gridStorageBuffer,
		std::unique_ptr<BufferData>& indirectDispatchBuffer)
		: vulkanContext(vulkanContext),
		fluidUniform(fluidUniform), indirectDispatch(indirectDispatch),
		fluidUniformBuffer(fluidUniformBuffer), particlesStorageBuffer(particlesStorageBuffer),
		particlesStorageBufferCopy(particlesStorageBufferCopy), gridStorageBuffer(gridStorageBuffer),
		indirectDispatchBuffer(indirectDispatchBuffer)
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

	void Run(vk::CommandBuffer& cb, int imageIndex, int groupCountX, bool particles = false)
	{
		cb.bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline);
		cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, computePipelineLayout, 0, computeDescriptorSet->descriptorSets[imageIndex], {});

		if (particles)
		{
			size_t offset = (std::size_t)&indirectDispatch.particlesCountDispatch - (std::size_t)&indirectDispatch;
			cb.dispatchIndirect(indirectDispatchBuffer->buffer, offset);
		}
		else
		{
			cb.dispatch(groupCountX, 1, 1);
		}
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
	IndirectDispatch& indirectDispatch;
	std::unique_ptr<BufferData>& indirectDispatchBuffer;
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
		int dimention = 3;

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
		fluidUniform.gridCellSize = 1;
		fluidUniform.gridDimention = { 5, 5, 5 };
		fluidUniform.gridSize = fluidUniform.gridDimention * fluidUniform.gridCellSize;

		{
			auto bb = BoundingBox();
			bb.aa = -Vector3f(fluidUniform.gridDimention) * fluidUniform.gridCellSize / 2;
			bb.bb = Vector3f(fluidUniform.gridDimention) * fluidUniform.gridCellSize / 2;
			auto bbObject = std::make_unique<BoundingBoxObject>(vulkanContext, bb);
			bbObjects.push_back(std::move(bbObject));

			for (int z = 0; z < fluidUniform.gridDimention.z; ++z)
			{
				for (int y = 0; y < fluidUniform.gridDimention.y; ++y)
				{
					for (int x = 0; x < fluidUniform.gridDimention.z; ++x)
					{
						auto gridCellPos = Vector3f(x, y, z);
						auto centerShift = -Vector3f(fluidUniform.gridDimention) * fluidUniform.gridCellSize / 2;
						auto bb = BoundingBox();
						bb.aa = gridCellPos * fluidUniform.gridCellSize + centerShift;
						bb.bb = (gridCellPos + Vector3f(1, 1, 1)) * fluidUniform.gridCellSize + centerShift;
						auto bbObject = std::make_unique<BoundingBoxObject>(vulkanContext, bb);
						bbObjects.push_back(std::move(bbObject));
					}
				}
			}
		}

		auto grid = std::vector<GridCell>(fluidUniform.gridDimention.x * fluidUniform.gridDimention.y * fluidUniform.gridDimention.z);

		fluidUniformBuffer = BufferData::Create(
			vulkanContext, fluidUniform, MemoryType::Universal, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc);
		particlesStorageBuffer = BufferData::Create(
			vulkanContext, particles, MemoryType::Universal, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst);
		particlesStorageBufferCopy = BufferData::Create(
			vulkanContext, particles, MemoryType::Universal, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc);
		gridStorageBuffer = BufferData::Create(
			vulkanContext, grid, MemoryType::DeviceLocal, vk::BufferUsageFlagBits::eStorageBuffer);

		auto icosphere = GeometryCreator::CreateIcosphere(0.2, 1);
		auto fluidRenderer = std::make_unique<FluidRenderer>(vulkanContext);
		fluidRenderer->UpdateVertexBuffer(*icosphere);
		fluidRenderer->descriptorSets->UpdateStorageDescriptor(*particlesStorageBufferCopy, 3);
		fluidRenderer->descriptorSets->UpdateStorageDescriptor(*fluidUniformBuffer, 4);

		indirectDispatch.particlesCountDispatch = vk::DispatchIndirectCommand(fluidUniform.particlesCount, 1, 1);
		indirectDispatch.particlesCountDraw.instanceCount = fluidUniform.particlesCount;
		indirectDispatch.particlesCountDraw.vertexCount = fluidRenderer->vertexBuffer->count;
		indirectDispatchBuffer = BufferData::Create(
			vulkanContext, indirectDispatch, MemoryType::Universal,
			vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eTransferDst);

		renderer = std::move(fluidRenderer);

		clearGridProgram = std::make_unique<ComputeProgram>(vulkanContext,
			"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/Compute/fluid.comp", "clearGrid",
			fluidUniform, indirectDispatch,
			fluidUniformBuffer, particlesStorageBuffer, particlesStorageBufferCopy, gridStorageBuffer, indirectDispatchBuffer);
		determineGridCellsProgram = std::make_unique<ComputeProgram>(vulkanContext,
			"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/Compute/fluid.comp", "determineGridCells",
			fluidUniform, indirectDispatch,
			fluidUniformBuffer, particlesStorageBuffer, particlesStorageBufferCopy, gridStorageBuffer, indirectDispatchBuffer);
		countGridCellsOffsetProgram = std::make_unique<ComputeProgram>(vulkanContext,
			"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/Compute/fluid.comp", "countGridCellsOffset",
			fluidUniform, indirectDispatch,
			fluidUniformBuffer, particlesStorageBuffer, particlesStorageBufferCopy, gridStorageBuffer, indirectDispatchBuffer);
		distributeByCellsProgram = std::make_unique<ComputeProgram>(vulkanContext,
			"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/Compute/fluid.comp", "distributeByCells",
			fluidUniform, indirectDispatch,
			fluidUniformBuffer, particlesStorageBuffer, particlesStorageBufferCopy, gridStorageBuffer, indirectDispatchBuffer);
		moveParticlesProgram = std::make_unique<ComputeProgram>(vulkanContext,
			"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/Compute/fluid.comp", "moveParticles",
			fluidUniform, indirectDispatch,
			fluidUniformBuffer, particlesStorageBuffer, particlesStorageBufferCopy, gridStorageBuffer, indirectDispatchBuffer);
	}

	void Run(vk::CommandBuffer& cb, int imageIndex)
	{
		{
			vk::BufferCopy copyRegion(0, 0, particlesStorageBufferCopy->size);
			cb.copyBuffer(particlesStorageBufferCopy->buffer, particlesStorageBuffer->buffer, copyRegion);

			vk::BufferMemoryBarrier barrier(vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead,
				vulkanContext.queueFamilies->computeQueueFamily, vulkanContext.queueFamilies->computeQueueFamily,
				particlesStorageBuffer->buffer, 0, particlesStorageBuffer->size);
			cb.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eComputeShader, {}, {}, barrier, {});
		}

		clearGridProgram->Run(cb, imageIndex, fluidUniform.gridDimention.x * fluidUniform.gridDimention.y * fluidUniform.gridDimention.z);

		{
			vk::BufferMemoryBarrier barrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead,
				vulkanContext.queueFamilies->computeQueueFamily, vulkanContext.queueFamilies->computeQueueFamily,
				gridStorageBuffer->buffer, 0, gridStorageBuffer->size);
			cb.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, {}, {}, barrier, {});
		}

		determineGridCellsProgram->Run(cb, imageIndex, fluidUniform.particlesCount, true);

		{
			vk::BufferMemoryBarrier barrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead,
				vulkanContext.queueFamilies->computeQueueFamily, vulkanContext.queueFamilies->computeQueueFamily,
				gridStorageBuffer->buffer, 0, gridStorageBuffer->size);
			cb.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, {}, {}, barrier, {});
		}

		countGridCellsOffsetProgram->Run(cb, imageIndex, 1);

		{
			vk::BufferMemoryBarrier barrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead,
				vulkanContext.queueFamilies->computeQueueFamily, vulkanContext.queueFamilies->computeQueueFamily,
				gridStorageBuffer->buffer, 0, gridStorageBuffer->size);
			cb.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, {}, {}, barrier, {});
		}

		{
			vk::BufferMemoryBarrier barrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead,
				vulkanContext.queueFamilies->computeQueueFamily, vulkanContext.queueFamilies->computeQueueFamily,
				fluidUniformBuffer->buffer, 0, fluidUniformBuffer->size);
			cb.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, {}, {}, barrier, {});
		}

		distributeByCellsProgram->Run(cb, imageIndex, fluidUniform.particlesCount, true);

		{
			size_t size = sizeof(fluidUniform.particlesCount);
			size_t srcOffset = (std::size_t)&fluidUniform.particlesCount - (std::size_t)&fluidUniform;
			size_t dstDispatchOffset = (std::size_t)&indirectDispatch.particlesCountDispatch.x - (std::size_t)&indirectDispatch;
			size_t dstDrawOffset = (std::size_t)&indirectDispatch.particlesCountDraw.instanceCount - (std::size_t)&indirectDispatch;

			vk::BufferCopy copyDispatchRegion(srcOffset, dstDispatchOffset, size);
			vk::BufferCopy copyDrawRegion(srcOffset, dstDrawOffset, size);
			cb.copyBuffer(fluidUniformBuffer->buffer, indirectDispatchBuffer->buffer, { copyDispatchRegion, copyDrawRegion });

			vk::BufferMemoryBarrier barrier(vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead,
				vulkanContext.queueFamilies->computeQueueFamily, vulkanContext.queueFamilies->computeQueueFamily,
				indirectDispatchBuffer->buffer, 0, indirectDispatchBuffer->size);
			cb.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eComputeShader, {}, {}, barrier, {});
		}

		{
			vk::BufferMemoryBarrier barrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead,
				vulkanContext.queueFamilies->computeQueueFamily, vulkanContext.queueFamilies->computeQueueFamily,
				particlesStorageBufferCopy->buffer, 0, particlesStorageBufferCopy->size);
			cb.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, {}, {}, barrier, {});
		}

		moveParticlesProgram->Run(cb, imageIndex, fluidUniform.particlesCount, true);
	}

	virtual void Render(RenderVisitor& renderVisitor) override
	{
		for (auto& bbObject : bbObjects) {
			bbObject->Render(renderVisitor);
		}

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

		size_t offset = (std::size_t)&indirectDispatch.particlesCountDraw - (std::size_t)&indirectDispatch;
		renderVisitor.commandBuffer.drawIndirect(indirectDispatchBuffer->buffer, offset, 1, 0);
	}

	virtual void Dispose() override
	{
		indirectDispatchBuffer->Dispose();
		fluidUniformBuffer->Dispose();
		particlesStorageBuffer->Dispose();
		particlesStorageBufferCopy->Dispose();
		gridStorageBuffer->Dispose();

		for (auto& bbObject : bbObjects) {
			bbObject->Dispose();
		}

		clearGridProgram->Dispose();
		determineGridCellsProgram->Dispose();
		countGridCellsOffsetProgram->Dispose();
		distributeByCellsProgram->Dispose();
		moveParticlesProgram->Dispose();

		Object::Dispose();
	}

	VulkanContext& vulkanContext;

	std::vector<Particle> particles;
	FluidUniform fluidUniform;
	IndirectDispatch indirectDispatch;

	std::vector<std::unique_ptr<BoundingBoxObject>> bbObjects;

	std::unique_ptr<BufferData> indirectDispatchBuffer;

	std::unique_ptr<BufferData> fluidUniformBuffer;
	std::unique_ptr<BufferData> particlesStorageBuffer;
	std::unique_ptr<BufferData> particlesStorageBufferCopy;
	std::unique_ptr<BufferData> gridStorageBuffer;

	std::unique_ptr<ComputeProgram> clearGridProgram;
	std::unique_ptr<ComputeProgram> determineGridCellsProgram;
	std::unique_ptr<ComputeProgram> countGridCellsOffsetProgram;
	std::unique_ptr<ComputeProgram> distributeByCellsProgram;
	std::unique_ptr<ComputeProgram> moveParticlesProgram;
};