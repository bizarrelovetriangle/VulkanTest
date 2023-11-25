#include "RenderObject.h"
#include "../../RenderVisitor.h"
#include "../../Vulkan/DescriptorSets.h"
#include "../../Utils/GLTFReader.h"
#include "../../Vulkan/Data/BufferData.h"
#include "../../VulkanContext.h"
#include "../../Utils/GLTFReader.h"
#include "../../Vulkan/Pipeline.h"
#include "../PlaneVertexedRenderObject.h"
#include "../ColoredRenderObject.h"
#include "../TexturedRenderObject.h"

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
std::shared_ptr<Shared<T>> Shared<T>::getInstance(VulkanContext& vulkanContext)
{
	auto ptr = instance.lock();
	if (!ptr) ptr = std::make_shared<Shared<T>>(vulkanContext);
	return ptr;
}

template <class T>
Shared<T>::Shared(VulkanContext& vulkanContext)
	: RenderObjectShared(vulkanContext)
{
	auto& device = vulkanContext.deviceController->device;
	auto bindings = T::DescriptorSetLayoutBinding();
	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreate({}, bindings);
	descriptorSetLayout = device.createDescriptorSetLayout(descriptorSetLayoutCreate);
	vertexDataBinding = T::VertexDataType::BindingDescription();
	vertexDataAttributes = T::VertexDataType::AttributeDescriptions();
	vertexShader = T::VertexShader;
	fragmentShader = T::FragmentShader;
	pipeline = std::make_unique<Pipeline>(vulkanContext, *this);
}

template <class T>
std::weak_ptr<Shared<T>> Shared<T>::instance;


RenderObject::RenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject)
{
	name = deserializedObject.name;
	model = deserializedObject.model;

	uniform.hasTexture = deserializedObject.textureData.has_value();
	uniform.hasColors = deserializedObject.hasColors;
	uniform.baseColor = deserializedObject.baseColor;

	std::span<RenderObjectUniform> uniformSpan(&uniform, &uniform + 1);
	uniformBuffer = std::make_unique<BufferData>(BufferData::Create<RenderObjectUniform>(
		vulkanContext, uniformSpan, MemoryType::Universal, vk::BufferUsageFlagBits::eUniformBuffer));
}

std::vector<vk::DescriptorSetLayoutBinding> RenderObject::DescriptorSetLayoutBinding()
{
	return {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll) 
	};
}

RenderObject::~RenderObject() = default;

void RenderObject::Accept(RenderVisitor& renderVisitor) const
{
	renderVisitor.Visit(*this);
}

void RenderObject::Dispose()
{
	uniformBuffer->Dispose();
	descriptorSets->Dispose();
	shared.reset();
}

template Shared<PlaneVertexedRenderObject>;
template Shared<ColoredRenderObject>;
template Shared<TexturedRenderObject>;
