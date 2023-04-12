#include "DeviceMemory.h"
#include "../DeviceController.h"
#include "../VertexData.h"
#include "../../Primitives/RenderObject.h"
#include "../../VulkanContext.h"

DeviceMemory::DeviceMemory(VulkanContext& vulkanContext)
	: vulkanContext(vulkanContext)
{
}

void DeviceMemory::AllocateMemory(const vk::MemoryRequirements& memoryRequirements)
{
	auto memoryType = FindMemoryType(memoryRequirements.memoryTypeBits,
		vk::MemoryPropertyFlagBits::eDeviceLocal |
		vk::MemoryPropertyFlagBits::eHostVisible |
		vk::MemoryPropertyFlagBits::eHostCoherent);

	vk::MemoryAllocateInfo allocInfo(memoryRequirements.size, memoryType);
	memory = vulkanContext.deviceController->device.allocateMemory(allocInfo);;
}

template <class T>
void DeviceMemory::FlushData(const std::vector<T>& data)
{
	size_t size = sizeof(T) * data.size();
	auto dataPointer = vulkanContext.deviceController->device.mapMemory(memory, 0, size);
	memcpy(dataPointer, data.data(), size);
	vulkanContext.deviceController->device.unmapMemory(memory);
}

uint32_t DeviceMemory::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
	auto memProperties = vulkanContext.deviceController->physicalDevice.getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void DeviceMemory::Dispose()
{
	vulkanContext.deviceController->device.freeMemory(memory);
}

template void DeviceMemory::FlushData(const std::vector<RenderObjectVertexData>& data);
template void DeviceMemory::FlushData(const std::vector<VertexData>& data);
template void DeviceMemory::FlushData(const std::vector<uint16_t>& data);

