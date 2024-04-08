#include "RenderObject.h"
#include "../../RenderVisitor.h"
#include "../../Vulkan/DescriptorSets.h"
#include "../../Utils/GLTFReader.h"
#include "../../Vulkan/Data/BufferData.h"
#include "../../VulkanContext.h"
#include "../../Utils/GLTFReader.h"
#include "../../Vulkan/Pipeline.h"
#include "../SimpleVertexedRenderObject.h"
#include "../ColoredRenderObject.h"
#include "../TexturedRenderObject.h"
#include "../PlaneRenderObject.h"
#include "../../Objects/Primitives/BoundingBoxObject.h"
#include "../Interfaces/VertexedRenderObject.h"
#include "../LinedRenderObject.h"

RenderObjectShared::RenderObjectShared(VulkanContext& vulkanContext)
	: vulkanContext(vulkanContext)
{
}

RenderObjectShared::~RenderObjectShared()
{
	auto& device = vulkanContext.deviceController->device;
	device.destroyDescriptorSetLayout(descriptorSetLayout);
	pipeline->Dispose();
}

template <class T>
std::shared_ptr<Shared<T>> Shared<T>::getInstance(VulkanContext& vulkanContext, bool lined)
{
	auto ptr = instance.lock();
	if (!ptr) {
		ptr = std::make_shared<Shared<T>>(vulkanContext, lined);
		instance = ptr;
	}
	return std::move(ptr);
}

template <class T>
Shared<T>::Shared(VulkanContext& vulkanContext, bool lined)
	: RenderObjectShared(vulkanContext)
{
	auto& device = vulkanContext.deviceController->device;
	auto bindings = T::DescriptorSetLayoutBinding();
	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreate({}, bindings);
	descriptorSetLayout = device.createDescriptorSetLayout(descriptorSetLayoutCreate);

	if constexpr (std::is_base_of_v<VertexedRenderObject, T>)
	{
		vertexDataBindings = { T::VertexDataType::BindingDescription() };
		vertexDataAttributes = T::VertexDataType::AttributeDescriptions();
	}

	vertexShader = T::VertexShader;
	fragmentShader = T::FragmentShader;
	pipeline = std::make_unique<Pipeline>(vulkanContext, *this, lined);
}

template <class T>
std::weak_ptr<Shared<T>> Shared<T>::instance;


RenderObject::RenderObject(VulkanContext& vulkanContext)
	: vulkanContext(vulkanContext)
{
	std::span<TransformUniform> transformUniformSpan(&transformUniform, &transformUniform + 1);
	transformUniformBuffer = BufferData::Create<TransformUniform>(
		vulkanContext, transformUniformSpan, MemoryType::Universal, vk::BufferUsageFlagBits::eUniformBuffer);

	std::span<PropertiesUniform> propertiesUniformSpan(&propertiesUniform, &propertiesUniform + 1);
	propertiesUniformBuffer = BufferData::Create<PropertiesUniform>(
		vulkanContext, propertiesUniformSpan, MemoryType::Universal, vk::BufferUsageFlagBits::eUniformBuffer);
}

RenderObject::~RenderObject() = default;

void RenderObject::UpdateTransformUniformBuffer()
{
	std::span<TransformUniform> uniformSpan(&transformUniform, &transformUniform + 1);
	transformUniformBuffer->FlushData(uniformSpan);
}

void RenderObject::UpdatePropertiesUniformBuffer()
{
	std::span<PropertiesUniform> uniformSpan(&propertiesUniform, &propertiesUniform + 1);
	propertiesUniformBuffer->FlushData(uniformSpan);
}

std::vector<vk::DescriptorSetLayoutBinding> RenderObject::DescriptorSetLayoutBinding()
{
	return {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll)
	};
}

void RenderObject::Accept(RenderVisitor& renderVisitor, const Camera& camera)
{
	renderVisitor.Visit(*this, camera);
}

void RenderObject::Dispose()
{
	transformUniformBuffer->Dispose();
	propertiesUniformBuffer->Dispose();
	descriptorSets->Dispose();
	shared.reset();
}

template Shared<SimpleVertexedRenderObject>;
template Shared<LinedRenderObject>;
template Shared<ColoredRenderObject>;
template Shared<TexturedRenderObject>;
template Shared<PlaneRenderObject>;
