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
}

void CommandBufferDispatcher::SubmitFence(vk::Fence fence, std::function<void()> callback)
{
	pendingCallbacks.emplace_back(fence, callback);
}

void CommandBufferDispatcher::PullFences()
{
	auto device = vulkanContext.deviceController->device;

	for (auto it = pendingCallbacks.begin(); it != pendingCallbacks.end();)
	{
		auto& [fence, callback] = *it;

		// We can add the same fence two times and it will be allright
		if (!fence || device.getFenceStatus(fence) == vk::Result::eSuccess)
		{
			callback();
			if (fence)
				device.destroyFence(fence);
			it = pendingCallbacks.erase(it);
		}
		else {
			it = std::next(it);
		}
	}
}

void CommandBufferDispatcher::Invoke(uint32_t queueFamily, std::function<void(vk::CommandBuffer&)> command)
{
	auto device = vulkanContext.deviceController->device;
	auto fence = InvokeSync(queueFamily, command);
	auto _ = device.waitForFences(fence, true, UINT64_MAX);
}

vk::Fence CommandBufferDispatcher::InvokeSync(
	uint32_t queueFamily, std::function<void(vk::CommandBuffer&)> command,
	const std::vector<vk::Semaphore>& waitSemaphores, const std::vector<vk::PipelineStageFlags>& waitDstStageMask,
	const std::vector<vk::Semaphore>& signalSemaphores)
{
	auto device = vulkanContext.deviceController->device;

	if (auto it = commandStructs.emplace(queueFamily, CommandStruct{}); it.second) {
		vk::CommandPoolCreateInfo commandPoolInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueFamily);
		it.first->second.commandPool = device.createCommandPool(commandPoolInfo);
	}

	vk::CommandBuffer commandBuffer;

	auto& commandStruct = commandStructs.at(queueFamily);

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
	vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	commandBuffer.begin(beginInfo);
	command(commandBuffer);
	commandBuffer.end();

	vk::FenceCreateInfo fenceInfo;
	auto fence = device.createFence(fenceInfo);
	vk::SubmitInfo submitInfo(waitSemaphores, waitDstStageMask, commandBuffer, signalSemaphores);
	auto& queue = vulkanContext.queueFamilies->queueMap.at(queueFamily);
	queue.submit(submitInfo, fence);

	SubmitFence(fence, [&commandStruct, commandBuffer]() { commandStruct.commandBuffers.push_back(commandBuffer); });

	return fence;
}
