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
	auto device = vulkanContext.deviceController->device;
	PullFences();

	for (auto& [queueIndex, commandStruct] : commandStructs)
	{
		for (auto& commandBuffer : commandStruct.commandBuffers)
		{
			device.freeCommandBuffers(commandStruct.commandPool, commandBuffer);
		}

		device.destroyCommandPool(commandStruct.commandPool);
	}

	for (auto& fence : freeFences)
	{
		device.destroyFence(fence);
	}
}

void CommandBufferDispatcher::SubmitFence(vk::Fence fence, uint32_t queueFamily, vk::CommandBuffer commandBuffer)
{
	pendingCallbacks.emplace_back(fence, [this, fence, queueFamily, commandBuffer]()
		{
			auto& commandStruct = commandStructs.at(queueFamily);
			commandStruct.commandBuffers.push_back(commandBuffer);
			freeFences.push_back(fence);
		});
}

void CommandBufferDispatcher::SubmitFence(vk::Fence fence, std::function<void()> callback)
{
	pendingCallbacks.emplace_back(fence, std::move(callback));
}

void CommandBufferDispatcher::PullFences()
{
	auto device = vulkanContext.deviceController->device;

	for (auto it = pendingCallbacks.begin(); it != pendingCallbacks.end();)
	{
		auto& [fence, callback] = *it;

		if (device.getFenceStatus(fence) == vk::Result::eSuccess)
		{
			callback();
			it = pendingCallbacks.erase(it);
		}
		else {
			it = std::next(it);
		}
	}
}

vk::Fence CommandBufferDispatcher::GetFence()
{
	vk::Fence fence;

	if (!freeFences.empty())
	{
		fence = freeFences.back();
		freeFences.pop_back();
	}
	else
	{
		vk::FenceCreateInfo fenceInfo;
		fence = vulkanContext.deviceController->device.createFence(fenceInfo);
	}

	vulkanContext.deviceController->device.resetFences(fence);

	return fence;
}

vk::CommandBuffer CommandBufferDispatcher::GetCommandBuffer(uint32_t queueFamily)
{
	auto device = vulkanContext.deviceController->device;

	if (auto it = commandStructs.emplace(queueFamily, CommandStruct{}); it.second) {
		vk::CommandPoolCreateInfo commandPoolInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueFamily);
		it.first->second.commandPool = device.createCommandPool(commandPoolInfo);
	}

	auto& commandStruct = commandStructs.at(queueFamily);
	vk::CommandBuffer commandBuffer;

	if (!commandStruct.commandBuffers.empty())
	{
		commandBuffer = commandStruct.commandBuffers.back();
		commandStruct.commandBuffers.pop_back();
	}
	else
	{
		vk::CommandBufferAllocateInfo commandBufferInfo(commandStruct.commandPool, vk::CommandBufferLevel::ePrimary, 1);
		commandBuffer = device.allocateCommandBuffers(commandBufferInfo).front();
	}

	commandBuffer.reset();
	return commandBuffer;
}

void CommandBufferDispatcher::Invoke(uint32_t queueFamily, std::function<void(vk::CommandBuffer&)> command)
{
	auto commandBuffer = GetCommandBuffer(queueFamily);
	auto fence = GetFence();

	InvokeSync(queueFamily, command, commandBuffer, fence);
	auto device = vulkanContext.deviceController->device;

	auto _ = device.waitForFences(fence, true, UINT64_MAX);
	freeFences.push_back(fence);
	auto& commandStruct = commandStructs.at(queueFamily);
	commandStruct.commandBuffers.push_back(commandBuffer);
}

void CommandBufferDispatcher::InvokeSync(
	uint32_t queueFamily, std::function<void(vk::CommandBuffer&)> command,
	vk::CommandBuffer& commandBuffer, vk::Fence& fence,
	const std::vector<vk::Semaphore>& waitSemaphores, const std::vector<vk::PipelineStageFlags>& waitDstStageMask,
	const std::vector<vk::Semaphore>& signalSemaphores)
{
	auto device = vulkanContext.deviceController->device;

	vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	commandBuffer.begin(beginInfo);
	command(commandBuffer);
	commandBuffer.end();

	vk::SubmitInfo submitInfo(waitSemaphores, waitDstStageMask, commandBuffer, signalSemaphores);
	auto& queue = vulkanContext.queueFamilies->queueMap.at(queueFamily);
	queue.submit(submitInfo, fence);
}

