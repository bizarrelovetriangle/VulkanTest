#include "BufferData.h"
#include "../DeviceController.h"
#include "../../RenderObjects/Interfaces/RenderObject.h"
#include "../../VulkanContext.h"
#include "../QueueFamilies.h"
#include "../CommandBuffer.h"
#include "../CommandBufferDispatcher.h"
#include "../../RenderObjects/Interfaces/RenderObject.h"
#include "../../RenderObjects/TexturedRenderObject.h"
#include "../../RenderObjects/ColoredRenderObject.h"
#include "../../RenderObjects/SimpleVertexedRenderObject.h"
#include "../../RenderObjects/PlaneRenderObject.h"
#include "../../RenderObjects/LinedRenderObject.h"

template <class T>
std::unique_ptr<BufferData> BufferData::Create(VulkanContext& vulkanContext,
	std::span<T> data, MemoryType memoryType, vk::BufferUsageFlags usage)
{
	auto span = std::span<std::byte>((std::byte*)data.data(), (std::byte*)(data.data() + data.size()));
	auto bufferData = std::make_unique<BufferData>(vulkanContext, span.size(), memoryType, usage);
	bufferData->count = data.size();
	bufferData->FlushData<T>(data);
	return std::move(bufferData);
}

BufferData::BufferData(VulkanContext& vulkanContext,
	size_t size, MemoryType memoryType, vk::BufferUsageFlags usage)
	: vulkanContext(vulkanContext), deviceMemory(vulkanContext, memoryType)
{
	auto& device = vulkanContext.deviceController->device;

	if (memoryType == MemoryType::DeviceLocal) usage |= vk::BufferUsageFlagBits::eTransferDst;

	vk::BufferCreateInfo bufferInfo({}, size, usage, vk::SharingMode::eExclusive);
	buffer = device.createBuffer(bufferInfo);

	auto memoryRequirements = device.getBufferMemoryRequirements(buffer);
	deviceMemory.AllocateMemory(memoryRequirements);
	device.bindBufferMemory(buffer, deviceMemory.memory, 0);
}

template <class T>
void BufferData::FlushData(std::span<T> data)
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

void BufferData::Dispose()
{
	vulkanContext.deviceController->device.destroyBuffer(buffer);
	deviceMemory.Dispose();
}


template std::unique_ptr<BufferData> BufferData::Create<VertexData>(VulkanContext& vulkanContext,
	std::span<VertexData> data, MemoryType memoryType, vk::BufferUsageFlags usage);
template std::unique_ptr<BufferData> BufferData::Create<TexturedVertexData>(VulkanContext& vulkanContext,
	std::span<TexturedVertexData> data, MemoryType memoryType, vk::BufferUsageFlags usage);
template std::unique_ptr<BufferData> BufferData::Create<ColoredVertexData>(VulkanContext& vulkanContext,
	std::span<ColoredVertexData> data, MemoryType memoryType, vk::BufferUsageFlags usage);
template std::unique_ptr<BufferData> BufferData::Create<PropertiesUniform>(VulkanContext& vulkanContext,
	std::span<PropertiesUniform> data, MemoryType memoryType, vk::BufferUsageFlags usage);
template std::unique_ptr<BufferData> BufferData::Create<uint16_t>(VulkanContext& vulkanContext,
	std::span<uint16_t> data, MemoryType memoryType, vk::BufferUsageFlags usage);
template std::unique_ptr<BufferData> BufferData::Create<std::byte>(VulkanContext& vulkanContext,
	std::span<std::byte> data, MemoryType memoryType, vk::BufferUsageFlags usage);
template std::unique_ptr<BufferData> BufferData::Create<PlaneObjectUniform>(VulkanContext& vulkanContext,
	std::span<PlaneObjectUniform> data, MemoryType memoryType, vk::BufferUsageFlags usage);
template std::unique_ptr<BufferData> BufferData::Create<TransformUniform>(VulkanContext& vulkanContext,
	std::span<TransformUniform> data, MemoryType memoryType, vk::BufferUsageFlags usage);
template std::unique_ptr<BufferData> BufferData::Create<Vector3f>(VulkanContext& vulkanContext,
	std::span<Vector3f> data, MemoryType memoryType, vk::BufferUsageFlags usage);
template std::unique_ptr<BufferData> BufferData::Create<LinedVertexData>(VulkanContext& vulkanContext,
	std::span<LinedVertexData> data, MemoryType memoryType, vk::BufferUsageFlags usage);

template void BufferData::FlushData(std::span<VertexData> data);
template void BufferData::FlushData(std::span<TexturedVertexData> data);
template void BufferData::FlushData(std::span<ColoredVertexData> data);
template void BufferData::FlushData(std::span<PropertiesUniform> data);
template void BufferData::FlushData(std::span<uint16_t> data);
template void BufferData::FlushData(std::span<std::byte> data);
template void BufferData::FlushData(std::span<PlaneObjectUniform> data);
template void BufferData::FlushData(std::span<TransformUniform> data);
template void BufferData::FlushData(std::span<Vector3f> data);
template void BufferData::FlushData(std::span<LinedVertexData> data);

