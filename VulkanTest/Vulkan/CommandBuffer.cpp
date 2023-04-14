#pragma once
#include "CommandBuffer.h"
#include "DeviceController.h"
#include "QueueFamilies.h"
#include <memory>
#include "Pipeline.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "../Primitives/RenderObject.h"
#include "../RenderVisitor.h";
#include "../VulkanContext.h";

CommandBuffer::CommandBuffer(VulkanContext& vulkanContext,
	const vk::Device& device, std::shared_ptr<QueueFamilies> queueFamilies,
	std::shared_ptr<Pipeline> pipeline, std::shared_ptr<SwapChain> swapChain,
	std::shared_ptr<RenderPass> renderPass)
	: vulkanContext(vulkanContext), device(device), queueFamilies(queueFamilies), pipeline(pipeline), swapChain(swapChain), renderPass(renderPass)
{
	CreateCommandPool();
	CreateCommandBuffer();
}

void CommandBuffer::Dispose()
{
	device.destroyCommandPool(commandPool);
}

void CommandBuffer::RecordCommandBuffer(size_t imageIndex,
	const std::vector<std::unique_ptr<RenderObject>>& renderObjects)
{
	vk::CommandBufferBeginInfo beginInfo;
	commandBuffer.begin(beginInfo);

	{
		vk::Rect2D renderArea({ 0, 0 }, swapChain->swapChainExtent);

		vk::ClearColorValue clearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
		vk::ClearValue clearColor(clearColorValue);

		vk::RenderPassBeginInfo renderPassInfo(
			renderPass->renderPass, swapChain->swapChainFramebuffers[imageIndex],
			renderArea, clearColor);

		commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

		RenderVisitor renderVisitor(*this, *pipeline, imageIndex);

		for (auto& renderObject : renderObjects)
		{
			renderObject->Accept(renderVisitor);
		}

		commandBuffer.endRenderPass();
	}

	commandBuffer.end();
}

void CommandBuffer::CommandBuffer::Reset() {
	commandBuffer.reset();
}

void CommandBuffer::CreateCommandPool()
{
	vk::CommandPoolCreateInfo poolInfo(
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		vulkanContext.queueFamilies->graphicQueueFamily);

	commandPool = device.createCommandPool(poolInfo);
}

void CommandBuffer::CreateCommandBuffer()
{
	vk::CommandBufferAllocateInfo allocInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1);
	commandBuffer = device.allocateCommandBuffers(allocInfo).front();
}
