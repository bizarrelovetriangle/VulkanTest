#include "CommandBufferDispatcher.h"
#include "../VulkanContext.h"
#include "DeviceController.h"
#include "QueueFamilies.h"

CommandBufferDispatcher::CommandBufferDispatcher(VulkanContext& vulkanContext)
	: vulkanContext(vulkanContext)
{
}

void CommandBufferDispatcher::Dispose()
{
	for (auto& [queueFamily, commandPool] : commandPoolMap) 
		vulkanContext.deviceController->device.destroyCommandPool(commandPool);
}

void CommandBufferDispatcher::Invoke(uint32_t queueFamily, std::function<void(vk::CommandBuffer&)> command)
{
	if (auto it = commandPoolMap.emplace(queueFamily, vk::CommandPool()); it.second) {
		vk::CommandPoolCreateInfo commandPoolInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueFamily);
		auto commandPool = vulkanContext.deviceController->device.createCommandPool(commandPoolInfo);
		it.first->second = std::move(commandPool);
	}

	auto& commandPool = commandPoolMap.at(queueFamily);
	vk::CommandBufferAllocateInfo commandBufferInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1);
	auto commandBuffer = vulkanContext.deviceController->device.allocateCommandBuffers(commandBufferInfo).front();

	vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	commandBuffer.begin(beginInfo);
	command(commandBuffer);
	commandBuffer.end();

	vk::SubmitInfo submitInfo({}, {}, commandBuffer, {});
	auto& queue = vulkanContext.queueFamilies->queueMap.at(queueFamily);
	queue.submit(submitInfo);
	queue.waitIdle();

	vulkanContext.deviceController->device.freeCommandBuffers(commandPool, commandBuffer);
}