#include "BufferData.h"
#include "../DeviceController.h"

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

void BufferData::Dispose()
{
	uint32_t transferQueueFamily = vulkanContext.queueFamilies->transferQueueFamily;
	auto& queue = vulkanContext.queueFamilies->queueMap.at(transferQueueFamily);
	queue.waitIdle();

	vulkanContext.deviceController->device.destroyBuffer(buffer);
	deviceMemory.Dispose();
}

