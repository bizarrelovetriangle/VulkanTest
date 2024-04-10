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

void RenderVisitor::Visit(Renderer& Renderer, const Camera& camera)
{
}

void RenderVisitor::Visit(PlaneRenderer& planeObject, const Camera& camera)
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

void RenderVisitor::Visit(VertexedRenderer& Renderer, const Camera& camera)
{
	auto& pipeline = *Renderer.shared->pipeline;
	BindPipeline(pipeline);

	commandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, pipeline.pipelineLayout, 0,
		Renderer.descriptorSets->descriptorSets[imageIndex], {});

	vk::Buffer vertexBuffers[] = { Renderer.vertexBuffer->buffer };
	vk::DeviceSize vertexOffsets[] = { 0 };
	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, vertexOffsets);

	Renderer.transformUniform.view = camera.view;
	Renderer.transformUniform.frustum = camera.proj;
	Renderer.UpdateTransformUniformBuffer();

	commandBuffer.draw(Renderer.vertexBuffer->count, 1, 0, 0);
}

void RenderVisitor::BindPipeline(Pipeline& pipeline)
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.graphicsPipeline);
}

