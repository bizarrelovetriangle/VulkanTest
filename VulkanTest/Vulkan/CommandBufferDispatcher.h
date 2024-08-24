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

	void InvokeSync(
		uint32_t queueFamily, std::function<void(vk::CommandBuffer&)> command,
		vk::CommandBuffer& commandBuffer, vk::Fence& fence,
		const std::vector<vk::Semaphore>& waitSemaphores = {}, const std::vector<vk::PipelineStageFlags>& waitDstStageMask = {},
		const std::vector<vk::Semaphore>& signalSemaphores = {});

	void SubmitFence(vk::Fence fence, uint32_t queueFamily, vk::CommandBuffer commandBuffer);
	void SubmitFence(vk::Fence fence, std::function<void()> callback);
	void PullFences();

	vk::Fence GetFence();
	vk::CommandBuffer GetCommandBuffer(uint32_t queueFamily);

	void Dispose();

private:
	struct CommandStruct
	{
		vk::CommandPool commandPool;
		std::vector<vk::CommandBuffer> commandBuffers;
	};

	VulkanContext& vulkanContext;
	std::unordered_map<uint32_t, CommandStruct> commandStructs;
	std::vector<vk::Fence> freeFences;

	std::vector<std::pair<vk::Fence, std::function<void()>>> pendingCallbacks;


	struct Hasher
	{
		size_t operator()(const vk::Fence& fence) const
		{
			auto v = (VkFence)fence;
			return std::hash<size_t>{}(size_t(v));
		}
	};
	
	std::unordered_map<vk::Fence, std::function<void()>, Hasher> pendingCallbacks2;
};

