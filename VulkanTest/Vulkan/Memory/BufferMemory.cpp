#include "BufferMemory.h"
#include "../DeviceController.h"
#include "../../Primitives/RenderObject.h"
#include "../../VulkanContext.h"
#include "../QueueFamilies.h"
#include "../CommandBuffer.h"
#include "../CommandBufferDispatcher.h"
#include "../../Primitives/TexturedRenderObject.h"
#include "../../Primitives/ColoredRenderObject.h"
#include "../../Primitives/VertexedRenderObject.h"

template <class T>
BufferMemory<T>::BufferMemory(VulkanContext& vulkanContext,
	std::span<T> data, MemoryType memoryType, vk::BufferUsageFlags usage)
	: DeviceMemory(vulkanContext, memoryType)
{
	auto& device = vulkanContext.deviceController->device;
	size_t size = sizeof(T) * data.size();

	if (memoryType == MemoryType::DeviceLocal) usage |= vk::BufferUsageFlagBits::eTransferDst;

	vk::BufferCreateInfo bufferInfo({}, size, usage, vk::SharingMode::eExclusive);
	buffer = device.createBuffer(bufferInfo);

	auto memoryRequirements = device.getBufferMemoryRequirements(buffer);
	AllocateMemory(memoryRequirements);
	device.bindBufferMemory(buffer, memory, 0);

	auto span = std::span<std::byte>((std::byte*)data.data(), (std::byte*)(data.data() + data.size()));
	FlushData(span);
	count = data.size();
}

template<class T>
void BufferMemory<T>::FlushData(std::span<std::byte> data)
{
	if (memoryType == MemoryType::Universal || memoryType == MemoryType::HostLocal)
	{
		DeviceMemory::FlushData(data);
		return;
	}

	BufferMemory<std::byte> stagingBuffer(vulkanContext, data, MemoryType::HostLocal, vk::BufferUsageFlagBits::eTransferSrc);

	uint32_t transferQueueFamily = vulkanContext.queueFamilies->transferQueueFamily;
	vulkanContext.commandBufferDispatcher->Invoke(transferQueueFamily,
		[this, &stagingBuffer](auto& cb)
		{
			vk::BufferCopy copyRegion(0, 0, stagingBuffer.count);
			cb.copyBuffer(stagingBuffer.buffer, buffer, copyRegion);
		});

	stagingBuffer.Dispose();
}

template <class T>
void BufferMemory<T>::Dispose()
{
	vulkanContext.deviceController->device.destroyBuffer(buffer);
	DeviceMemory::Dispose();
}

template BufferMemory<VertexData>;
template BufferMemory<TexturedVertexData>;
template BufferMemory<ColoredVertexData>;
template BufferMemory<RenderObjectUniform>;
template BufferMemory<uint16_t>;
template BufferMemory<std::byte>;
