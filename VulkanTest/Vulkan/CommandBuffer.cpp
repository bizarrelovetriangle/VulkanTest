#pragma once
#include "CommandBuffer.h"
#include "DeviceController.h"
#include "QueueFamilies.h"
#include <memory>
#include "Pipeline.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "../VulkanContext.h";

CommandBuffer::CommandBuffer(VulkanContext& vulkanContext,
	const vk::Device& device, std::shared_ptr<QueueFamilies> queueFamilies,
	std::shared_ptr<SwapChain> swapChain,
	std::shared_ptr<RenderPass> renderPass)
	: vulkanContext(vulkanContext), device(device), queueFamilies(queueFamilies), swapChain(swapChain), renderPass(renderPass)
{
	vk::CommandPoolCreateInfo poolInfo(
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		vulkanContext.queueFamilies->graphicQueueFamily);

	commandPool = device.createCommandPool(poolInfo);

	vk::CommandBufferAllocateInfo allocInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1);
	commandBuffer = device.allocateCommandBuffers(allocInfo).front();
}

void CommandBuffer::Dispose()
{
	device.destroyCommandPool(commandPool);
}
