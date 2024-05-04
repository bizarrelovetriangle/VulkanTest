#include <vulkan/vulkan.hpp>
#include "RenderVisitor.h"
#include "Renderers/Interfaces/Renderer.h"
#include "Renderers/ColoredRenderer.h"
#include "Renderers/Interfaces/VertexedRenderer.h"
#include "Renderers/TexturedRenderer.h"
#include "Objects/Primitives/PlaneObject.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/DescriptorSets.h"
#include "Vulkan/SwapChain.h"
#include "VulkanContext.h"
#include "Vulkan/Data/BufferData.h"
#include "Camera.h"
#include "Renderers/PlaneRenderer.h"

RenderVisitor::RenderVisitor(VulkanContext& vulkanContext, CommandBuffer& commandBuffer, size_t imageIndex)
	: vulkanContext(vulkanContext), commandBuffer(commandBuffer.commandBuffer), imageIndex(imageIndex)
{
}

void RenderVisitor::Visit(Renderer& renderer)
{
}

void RenderVisitor::Visit(VertexedRenderer& renderer)
{
	auto& pipeline = *renderer.shared->pipeline;
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.graphicsPipeline);

	commandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, pipeline.pipelineLayout, 0,
		renderer.descriptorSets->descriptorSets[imageIndex], {});

	vk::Buffer vertexBuffers[] = { renderer.vertexBuffer->buffer };
	vk::DeviceSize vertexOffsets[] = { 0 };
	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, vertexOffsets);

	commandBuffer.draw(renderer.vertexBuffer->count, 1, 0, 0);
}


