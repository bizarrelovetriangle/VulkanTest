#include "BufferMemory.h"
#include "../DeviceController.h"
#include "../VertexData.h"
#include "../../Primitives/RenderObject.h"
#include "../../VulkanContext.h"
#include "../QueueFamilies.h"
#include "../CommandBuffer.h"

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
void BufferMemory<T>::StagingFlush(std::span<std::byte> data)
{
	BufferMemory<std::byte> stagingBuffer(vulkanContext, data, MemoryType::HostLocal, vk::BufferUsageFlagBits::eTransferSrc);

	vk::CommandBufferAllocateInfo commandBufferAllocInfo(vulkanContext.commandBuffer->commandPool,
		vk::CommandBufferLevel::ePrimary, 1);

	auto commandBuffer = vulkanContext.deviceController->device.allocateCommandBuffers(commandBufferAllocInfo).front();

	vk::CommandBufferBeginInfo beginInfo;
	commandBuffer.begin(beginInfo);

	vk::BufferCopy copyRegion(0, 0, stagingBuffer.count);
	commandBuffer.copyBuffer(stagingBuffer.buffer, buffer, copyRegion);

	commandBuffer.end();

	vk::SubmitInfo submitInfo({}, {}, commandBuffer);

	vulkanContext.queueFamilies->transferQueue.submit(submitInfo);
	vulkanContext.queueFamilies->transferQueue.waitIdle();

	vulkanContext.deviceController->device.freeCommandBuffers(vulkanContext.commandBuffer->commandPool, commandBuffer);

	stagingBuffer.Dispose();
}

template <class T>
void BufferMemory<T>::Dispose()
{
	vulkanContext.deviceController->device.destroyBuffer(buffer);
	DeviceMemory::Dispose();
}

template BufferMemory<RenderObjectVertexData>;
template BufferMemory<VertexData>;
template BufferMemory<uint16_t>;
template BufferMemory<std::byte>;
