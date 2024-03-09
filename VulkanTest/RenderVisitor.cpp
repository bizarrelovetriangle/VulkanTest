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
#include "RenderObjects/PlaneRenderObject.h"
#include <chrono>

RenderVisitor::RenderVisitor(VulkanContext& vulkanContext, CommandBuffer& commandBuffer, size_t imageIndex)
	: vulkanContext(vulkanContext), commandBuffer(commandBuffer.commandBuffer), imageIndex(imageIndex)
{
}

void RenderVisitor::Visit(RenderObject& renderObject)
{
}

void RenderVisitor::Visit(PlaneRenderObject& planeObject)
{
	auto& pipeline = *planeObject.shared->pipeline;
	BindPipeline(pipeline);

	commandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, pipeline.pipelineLayout, 0,
		planeObject.descriptorSets->descriptorSets[imageIndex], {});

	static auto before = std::chrono::high_resolution_clock::now();
	auto now = std::chrono::high_resolution_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - before);
	float seconds = float(diff.count()) / 1000;

	float degen = float(0. + seconds / 2);

	Matrix4 view;
	//view = Matrix4::RotateY(degen) * view;
	view = Matrix4::Translation(Vector3f(0, 0, 5)) * view;

	Matrix4 frustum = Matrix4::Frustum(0.1, 10, 1);

	planeObject.transformUniform.view = view;
	planeObject.transformUniform.frustum = frustum;
	planeObject.UpdateTransformUniformBuffer();

	commandBuffer.draw(6, 1, 0, 0);
}

void RenderVisitor::Visit(VertexedRenderObject& renderObject)
{
	auto& pipeline = *renderObject.shared->pipeline;
	BindPipeline(pipeline);

	commandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, pipeline.pipelineLayout, 0,
		renderObject.descriptorSets->descriptorSets[imageIndex], {});

	vk::Buffer vertexBuffers[] = { renderObject.vertexBuffer->buffer };
	vk::DeviceSize vertexOffsets[] = { 0 };
	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, vertexOffsets);

	static auto before = std::chrono::high_resolution_clock::now();
	auto now = std::chrono::high_resolution_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - before);
	float seconds = float(diff.count()) / 1000;

	float degen = -float(0. + seconds / 8);

	Matrix4 view;
	//view = Matrix4::RotateY(degen) * view;
	view = Matrix4::Translation(Vector3f(0, 0, 5)) * view;

	Matrix4 frustum = Matrix4::Frustum(0.1, 10, 1);

	renderObject.transformUniform.view = view;
	renderObject.transformUniform.frustum = frustum;
	renderObject.UpdateTransformUniformBuffer();

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

