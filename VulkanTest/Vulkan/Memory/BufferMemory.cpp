#include "BufferMemory.h"
#include "../DeviceController.h"
#include "../VertexData.h"
#include "../../Primitives/RenderObject.h"
#include "../../VulkanContext.h"

template <class T>
BufferMemory<T>::BufferMemory(VulkanContext& vulkanContext,
	const std::vector<T>& data, vk::BufferUsageFlagBits usage)
	: DeviceMemory(vulkanContext)
{
	auto& device = vulkanContext.deviceController->device;
	size_t size = sizeof(T) * data.size();

	vk::BufferCreateInfo bufferInfo({}, size, usage, vk::SharingMode::eExclusive);
	buffer = device.createBuffer(bufferInfo);

	auto memoryRequirements = device.getBufferMemoryRequirements(buffer);
	AllocateMemory(memoryRequirements);
	device.bindBufferMemory(buffer, memory, 0);

	FlushData(data);
	count = data.size();
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
