#include <vulkan/vulkan.hpp>
#include "RenderVisitor.h"
#include "Primitives/RenderObject.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/DescriptorSets.h"
#include "Vulkan/PipelineProvider.h"
#include "Vulkan/SwapChain.h"
#include "VulkanContext.h"
#include "Vulkan/Memory/BufferMemory.h"
#include "Vulkan/Memory/BufferMemory.h"

RenderVisitor::RenderVisitor(VulkanContext& vulkanContext, CommandBuffer& commandBuffer, size_t imageIndex)
	: vulkanContext(vulkanContext), commandBuffer(commandBuffer.commandBuffer), imageIndex(imageIndex)
{
}

void RenderVisitor::Visit(const RenderObject& renderObject)
{
	auto& pipeline = *vulkanContext.pipelineProvider->GetPipeline(renderObject);
	BindPipeline(pipeline);

	commandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, pipeline.pipelineLayout, 0, renderObject.descriptorSets->descriptorSets[imageIndex], {});

	vk::Buffer vertexBuffers[] = { renderObject.vertexBuffer->buffer };
	vk::DeviceSize vertexOffsets[] = { 0 };
	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, vertexOffsets);

	static auto before = std::chrono::high_resolution_clock::now();
	auto now = std::chrono::high_resolution_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - before);
	float seconds = float(diff.count()) / 1000;
	
	float degen = float(0. + seconds / 2);

	Matrix4 world;
	world = Matrix4::RotateY(degen) * world;
	world = Matrix4::Scale({ 0.3, 0.3, 0.3 }) * world;
	world = Matrix4::Translation(Vector3f(0, 0, 2)) * world;

	world.j *= -1;

	RenderObjectPushConstantRange pushConstantRange{ renderObject.model.Transpose(), world.Transpose() };
	commandBuffer.pushConstants(
		pipeline.pipelineLayout, vk::ShaderStageFlagBits::eVertex,
		0, sizeof(RenderObjectPushConstantRange), &pushConstantRange);

	commandBuffer.draw(renderObject.vertexBuffer->count, 1, 0, 0);
}

void RenderVisitor::BindPipeline(Pipeline& pipeline)
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.graphicsPipeline);
	auto viewport = vulkanContext.swapChain->CreateViewport();
	auto scissors = vulkanContext.swapChain->CreateScissors();
	commandBuffer.setViewport(0, 1, &viewport);
	commandBuffer.setScissor(0, 1, &scissors);
}
