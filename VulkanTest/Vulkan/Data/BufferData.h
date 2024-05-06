#pragma once
#include <memory>
#include <vulkan/vulkan.hpp>
#include "DeviceMemory/DeviceMemory.h"
#include "../QueueFamilies.h"
#include "../CommandBufferDispatcher.h"
#include "../../VulkanContext.h"

class BufferData
{
public:
	template <class T>
	static std::unique_ptr<BufferData> Create(VulkanContext& vulkanContext,
		std::span<T> data, MemoryType memoryType, vk::BufferUsageFlags usage)
	{
		auto span = std::span<std::byte>((std::byte*)data.data(), (std::byte*)(data.data() + data.size()));
		auto bufferData = std::make_unique<BufferData>(vulkanContext, span.size(), memoryType, usage);
		bufferData->count = data.size();
		bufferData->FlushData<T>(data);
		return std::move(bufferData);
	}

	template <class T>
	void FlushData(std::span<T> data)
	{
		if (deviceMemory.memoryType == MemoryType::Universal || deviceMemory.memoryType == MemoryType::HostLocal)
		{
			auto span = std::span<std::byte>((std::byte*)data.data(), (std::byte*)(data.data() + data.size()));
			deviceMemory.FlushMemory(span);
			return;
		}

		auto stagingBuffer = BufferData::Create<T>(
			vulkanContext, data, MemoryType::HostLocal, vk::BufferUsageFlagBits::eTransferSrc);

		uint32_t transferQueueFamily = vulkanContext.queueFamilies->transferQueueFamily;
		vulkanContext.commandBufferDispatcher->Invoke(transferQueueFamily,
			[this, &stagingBuffer](auto& cb)
			{
				vk::BufferCopy copyRegion(0, 0, stagingBuffer->count * sizeof(T));
				cb.copyBuffer(stagingBuffer->buffer, buffer, copyRegion);
			});

		stagingBuffer->Dispose();
	}

	template <class T>
	static std::unique_ptr<BufferData> Create(VulkanContext& vulkanContext,
		T& data, MemoryType memoryType, vk::BufferUsageFlags usage)
	{
		return Create<T>(vulkanContext, std::span<T>(&data, &data + 1), memoryType, usage);
	}

	template <class T>
	void FlushData(T& data)
	{
		FlushData(std::span<T>(&data, &data + 1));
	}

	BufferData(VulkanContext& vulkanContext,
		size_t size, MemoryType memoryType, vk::BufferUsageFlags usage);
	void Dispose();

public:
	size_t count = 0;
	vk::Buffer buffer;

private:
	VulkanContext& vulkanContext;
	DeviceMemory deviceMemory;
};
