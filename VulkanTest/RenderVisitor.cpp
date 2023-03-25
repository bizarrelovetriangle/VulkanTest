#include <vulkan/vulkan.hpp>
#include "RenderVisitor.h"
#include "Primitives/RenderObject.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Pipeline.h"

RenderVisitor::RenderVisitor(CommandBuffer& commandBuffer, Pipeline& pipeline)
	: commandBuffer(commandBuffer.commandBuffer), pipeline(pipeline)
{
}

void RenderVisitor::Visit(const RenderObject& renderObject)
{
	BindPipeline(pipeline);

	vk::Buffer vertexBuffers[] = { renderObject.vertexBuffer->buffer };
	vk::DeviceSize vertexOffsets[] = { 0 };
	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, vertexOffsets);

	Matrix4 world = Matrix4::Translation({ 0., 0., 0.5 }) * Matrix4::Scale({ 0.3, 0.3, 0.3 });

	RenderObjectPushConstantRange pushConstantRange{ renderObject.model.Transpose(), world.Transpose() };
	commandBuffer.pushConstants(
		pipeline.pipelineLayout, vk::ShaderStageFlagBits::eVertex,
		0, sizeof(RenderObjectPushConstantRange), &pushConstantRange);

	commandBuffer.draw(renderObject.vertexBuffer->count, 1, 0, 0);
}

void RenderVisitor::BindPipeline(Pipeline& pipeline)
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.graphicsPipeline);
	auto viewport = pipeline.CreateViewport();
	auto scissors = pipeline.CreateScissors();
	commandBuffer.setViewport(0, 1, &viewport);
	commandBuffer.setScissor(0, 1, &scissors);
}
