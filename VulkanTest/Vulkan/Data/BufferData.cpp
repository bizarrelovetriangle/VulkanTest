#include "BufferData.h"
#include "../DeviceController.h"
#include "../../Primitives/Interfaces/RenderObject.h"
#include "../../VulkanContext.h"
#include "../QueueFamilies.h"
#include "../CommandBuffer.h"
#include "../CommandBufferDispatcher.h"
#include "../../Primitives/TexturedRenderObject.h"
#include "../../Primitives/ColoredRenderObject.h"
#include "../../Primitives/PlaneVertexedRenderObject.h"

template <class T>
BufferData BufferData::Create(VulkanContext& vulkanContext,
	std::span<T> data, MemoryType memoryType, vk::BufferUsageFlags usage)
{
	auto span = std::span<std::byte>((std::byte*)data.data(), (std::byte*)(data.data() + data.size()));
	return BufferData(vulkanContext, span, memoryType, usage);
}

BufferData::BufferData(const BufferData& bufferData) = default;

BufferData::BufferData(VulkanContext& vulkanContext,
	std::span<std::byte> data, MemoryType memoryType, vk::BufferUsageFlags usage)
	: DeviceMemory(vulkanContext, memoryType)
{
	auto& device = vulkanContext.deviceController->device;

	if (memoryType == MemoryType::DeviceLocal) usage |= vk::BufferUsageFlagBits::eTransferDst;

	vk::BufferCreateInfo bufferInfo({}, data.size(), usage, vk::SharingMode::eExclusive);
	buffer = device.createBuffer(bufferInfo);

	auto memoryRequirements = device.getBufferMemoryRequirements(buffer);
	AllocateMemory(memoryRequirements);
	device.bindBufferMemory(buffer, memory, 0);

	auto span = std::span<std::byte>((std::byte*)data.data(), (std::byte*)(data.data() + data.size()));
	FlushData(span);
	count = data.size();
}

void BufferData::FlushData(std::span<std::byte> data)
{
	if (memoryType == MemoryType::Universal || memoryType == MemoryType::HostLocal)
	{
		DeviceMemory::FlushData(data);
		return;
	}

	BufferData stagingBuffer = BufferData::Create<std::byte>(
		vulkanContext, data, MemoryType::HostLocal, vk::BufferUsageFlagBits::eTransferSrc);

	uint32_t transferQueueFamily = vulkanContext.queueFamilies->transferQueueFamily;
	vulkanContext.commandBufferDispatcher->Invoke(transferQueueFamily,
		[this, &stagingBuffer](auto& cb)
		{
			vk::BufferCopy copyRegion(0, 0, stagingBuffer.count);
			cb.copyBuffer(stagingBuffer.buffer, buffer, copyRegion);
		});

	stagingBuffer.Dispose();
}

void BufferData::Dispose()
{
	vulkanContext.deviceController->device.destroyBuffer(buffer);
	DeviceMemory::Dispose();
}

template BufferData BufferData::Create<VertexData>(VulkanContext& vulkanContext,
	std::span<VertexData> data, MemoryType memoryType, vk::BufferUsageFlags usage);
template BufferData BufferData::Create<TexturedVertexData>(VulkanContext& vulkanContext,
	std::span<TexturedVertexData> data, MemoryType memoryType, vk::BufferUsageFlags usage);
template BufferData BufferData::Create<ColoredVertexData>(VulkanContext& vulkanContext,
	std::span<ColoredVertexData> data, MemoryType memoryType, vk::BufferUsageFlags usage);
template BufferData BufferData::Create<RenderObjectUniform>(VulkanContext& vulkanContext,
	std::span<RenderObjectUniform> data, MemoryType memoryType, vk::BufferUsageFlags usage);
template BufferData BufferData::Create<uint16_t>(VulkanContext& vulkanContext,
	std::span<uint16_t> data, MemoryType memoryType, vk::BufferUsageFlags usage);
template BufferData BufferData::Create<std::byte>(VulkanContext& vulkanContext,
	std::span<std::byte> data, MemoryType memoryType, vk::BufferUsageFlags usage);
