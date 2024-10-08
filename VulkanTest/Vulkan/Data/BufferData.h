#pragma once
#include <memory>
#include <vulkan/vulkan.hpp>
#include "DeviceMemory/DeviceMemory.h"
#include "../QueueFamilies.h"
#include "../CommandBufferDispatcher.h"
#include "../../VulkanContext.h"
#include "../DeviceController.h"

class BufferData
{
public:
	template <class T>
	static std::unique_ptr<BufferData> Create(VulkanContext& vulkanContext,
		std::span<T> data, MemoryType memoryType, vk::BufferUsageFlags usage, size_t reserveCount = 0)
	{
		if (reserveCount == 0) reserveCount = data.size();
		auto bufferData = std::make_unique<BufferData>(vulkanContext, reserveCount * sizeof(T), memoryType, usage);
		bufferData->reservedCount = reserveCount;
		bufferData->FlushData<T>(data);
		return bufferData;
	}

	template <class T>
	void FlushData(std::span<T> data)
	{
		if (data.size() > reservedCount)
		{
			auto extended = Create<T>(vulkanContext, data, deviceMemory.memoryType, usage, data.size() * 1.5);
			std::swap(*this, *extended);
			extended->Dispose();
			return;
		}

		count = data.size();

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

	BufferData(VulkanContext& vulkanContext,
		size_t size, MemoryType memoryType, vk::BufferUsageFlags usage)
		: vulkanContext(vulkanContext), deviceMemory(vulkanContext, memoryType), usage(usage)
	{
		auto& device = vulkanContext.deviceController->device;

		if (memoryType == MemoryType::DeviceLocal) usage |= vk::BufferUsageFlagBits::eTransferDst;

		vk::BufferCreateInfo bufferInfo({}, size, usage, vk::SharingMode::eExclusive);
		buffer = device.createBuffer(bufferInfo);

		auto memoryRequirements = device.getBufferMemoryRequirements(buffer);
		deviceMemory.AllocateMemory(memoryRequirements);
		device.bindBufferMemory(buffer, deviceMemory.memory, 0);
	}

	BufferData& operator=(const BufferData& bufferData)
	{
		deviceMemory = bufferData.deviceMemory;
		buffer = bufferData.buffer;
		usage = bufferData.usage;
		reservedCount = bufferData.reservedCount;
		count = bufferData.count;
		return *this;
	}

	void Dispose()
	{
		uint32_t transferQueueFamily = vulkanContext.queueFamilies->transferQueueFamily;
		auto& queue = vulkanContext.queueFamilies->queueMap.at(transferQueueFamily);
		queue.waitIdle();

		vulkanContext.deviceController->device.destroyBuffer(buffer);
		deviceMemory.Dispose();
	}

public:
	size_t count = 0;
	size_t reservedCount = 0;
	vk::Buffer buffer;

private:
	VulkanContext& vulkanContext;
	DeviceMemory deviceMemory;
	vk::BufferUsageFlags usage;
};
