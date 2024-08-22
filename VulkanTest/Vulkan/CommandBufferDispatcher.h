#pragma once
#include <vulkan/vulkan.hpp>
#include <functional>
#include <unordered_map>

class VulkanContext;

class CommandBufferDispatcher
{
public:
	CommandBufferDispatcher(VulkanContext& vulkanContext);

	void Invoke(uint32_t queueFamily, std::function<void(vk::CommandBuffer&)> command);
	vk::Fence InvokeSync(
		uint32_t queueFamily, std::function<void(vk::CommandBuffer&)> command,
		const std::vector<vk::Semaphore>& waitSemaphores = {}, const std::vector<vk::PipelineStageFlags>& waitDstStageMask = {},
		const std::vector<vk::Semaphore>& signalSemaphores = {});

	void SubmitFence(vk::Fence fence, std::function<void()> callback);
	void PullFences();

	void Dispose();

private:
	struct CommandStruct
	{
		vk::CommandPool commandPool;
		std::vector<vk::CommandBuffer> commandBuffers;
	};

	VulkanContext& vulkanContext;
	std::unordered_map<uint32_t, CommandStruct> commandStructs;

	std::vector<std::pair<vk::Fence, std::function<void()>>> pendingCallbacks;
};

