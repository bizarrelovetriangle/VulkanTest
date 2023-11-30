#include "BufferData.h"
#include "../DeviceController.h"
#include "../../RenderObjects/Interfaces/RenderObject.h"
#include "../../VulkanContext.h"
#include "../QueueFamilies.h"
#include "../CommandBuffer.h"
#include "../CommandBufferDispatcher.h"
#include "../../RenderObjects/Interfaces/RenderObject.h"
#include "../../RenderObjects/DeserializableObjects/TexturedRenderObject.h"
#include "../../RenderObjects/DeserializableObjects/ColoredRenderObject.h"
#include "../../RenderObjects/DeserializableObjects/PlaneVertexedRenderObject.h"
#include "../../RenderObjects/Primitives/EvenPlaneObject.h"

template <class T>
BufferData BufferData::Create(VulkanContext& vulkanContext,
	std::span<T> data, MemoryType memoryType, vk::BufferUsageFlags usage)
{
	auto span = std::span<std::byte>((std::byte*)data.data(), (std::byte*)(data.data() + data.size()));
	BufferData bufferData(vulkanContext, span.size(), memoryType, usage);
	bufferData.count = data.size();
	bufferData.FlushData(span);
	return bufferData;
}

BufferData::BufferData(const BufferData& bufferData) = default;

BufferData::BufferData(VulkanContext& vulkanContext,
	size_t size, MemoryType memoryType, vk::BufferUsageFlags usage)
	: DeviceMemory(vulkanContext, memoryType)
{
	auto& device = vulkanContext.deviceController->device;

	if (memoryType == MemoryType::DeviceLocal) usage |= vk::BufferUsageFlagBits::eTransferDst;

	vk::BufferCreateInfo bufferInfo({}, size, usage, vk::SharingMode::eExclusive);
	buffer = device.createBuffer(bufferInfo);

	auto memoryRequirements = device.getBufferMemoryRequirements(buffer);
	AllocateMemory(memoryRequirements);
	device.bindBufferMemory(buffer, memory, 0);
}

template <class T>
void BufferData::FlushData(std::span<T> data)
{
	auto span = std::span<std::byte>((std::byte*)data.data(), (std::byte*)(data.data() + data.size()));

	if (memoryType == MemoryType::Universal || memoryType == MemoryType::HostLocal)
	{
		FlushMemory(span);
		return;
	}

	BufferData stagingBuffer = BufferData::Create<std::byte>(
		vulkanContext, span, MemoryType::HostLocal, vk::BufferUsageFlagBits::eTransferSrc);

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
template BufferData BufferData::Create<DeserializableObjectUniform>(VulkanContext& vulkanContext,
	std::span<DeserializableObjectUniform> data, MemoryType memoryType, vk::BufferUsageFlags usage);
template BufferData BufferData::Create<uint16_t>(VulkanContext& vulkanContext,
	std::span<uint16_t> data, MemoryType memoryType, vk::BufferUsageFlags usage);
template BufferData BufferData::Create<std::byte>(VulkanContext& vulkanContext,
	std::span<std::byte> data, MemoryType memoryType, vk::BufferUsageFlags usage);
template BufferData BufferData::Create<EvenPlaneObjectUniform>(VulkanContext& vulkanContext,
	std::span<EvenPlaneObjectUniform> data, MemoryType memoryType, vk::BufferUsageFlags usage);
template BufferData BufferData::Create<TransformUniform>(VulkanContext& vulkanContext,
	std::span<TransformUniform> data, MemoryType memoryType, vk::BufferUsageFlags usage);

template void BufferData::FlushData(std::span<VertexData> data);
template void BufferData::FlushData(std::span<TexturedVertexData> data);
template void BufferData::FlushData(std::span<ColoredVertexData> data);
template void BufferData::FlushData(std::span<DeserializableObjectUniform> data);
template void BufferData::FlushData(std::span<uint16_t> data);
template void BufferData::FlushData(std::span<std::byte> data);
template void BufferData::FlushData(std::span<EvenPlaneObjectUniform> data);
template void BufferData::FlushData(std::span<TransformUniform> data);

