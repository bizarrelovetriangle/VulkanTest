#include <vulkan/vulkan.hpp>
#include "RenderVisitor.h"
#include "RenderObjects/Interfaces/RenderObject.h"
#include "RenderObjects/ColoredRenderObject.h"
#include "RenderObjects/Interfaces/VertexedRenderObject.h"
#include "RenderObjects/TexturedRenderObject.h"
#include "Objects/Primitives/PlaneObject.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/DescriptorSets.h"
#include "Vulkan/SwapChain.h"
#include "VulkanContext.h"
#include "Vulkan/Data/BufferData.h"
#include "Camera.h"
#include "RenderObjects/PlaneRenderObject.h"

RenderVisitor::RenderVisitor(VulkanContext& vulkanContext, CommandBuffer& commandBuffer, size_t imageIndex)
	: vulkanContext(vulkanContext), commandBuffer(commandBuffer.commandBuffer), imageIndex(imageIndex)
{
}

void RenderVisitor::Visit(RenderObject& renderObject, const Camera& camera)
{
}

void RenderVisitor::Visit(PlaneRenderObject& planeObject, const Camera& camera)
{
	auto& pipeline = *planeObject.shared->pipeline;
	BindPipeline(pipeline);

	commandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, pipeline.pipelineLayout, 0,
		planeObject.descriptorSets->descriptorSets[imageIndex], {});

	planeObject.transformUniform.view = camera.view;
	planeObject.transformUniform.frustum = camera.proj;
	planeObject.UpdateTransformUniformBuffer();

	commandBuffer.draw(6, 1, 0, 0);
}

void RenderVisitor::Visit(VertexedRenderObject& renderObject, const Camera& camera)
{
	auto& pipeline = *renderObject.shared->pipeline;
	BindPipeline(pipeline);

	commandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, pipeline.pipelineLayout, 0,
		renderObject.descriptorSets->descriptorSets[imageIndex], {});

	vk::Buffer vertexBuffers[] = { renderObject.vertexBuffer->buffer };
	vk::DeviceSize vertexOffsets[] = { 0 };
	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, vertexOffsets);

	renderObject.transformUniform.view = camera.view;
	renderObject.transformUniform.frustum = camera.proj;
	renderObject.UpdateTransformUniformBuffer();

	commandBuffer.draw(renderObject.vertexBuffer->count, 1, 0, 0);
}

void RenderVisitor::BindPipeline(Pipeline& pipeline)
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.graphicsPipeline);
}

