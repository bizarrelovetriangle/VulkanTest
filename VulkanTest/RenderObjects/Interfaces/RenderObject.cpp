#include "RenderObject.h"
#include "../../RenderVisitor.h"
#include "../../Vulkan/DescriptorSets.h"
#include "../../Utils/GLTFReader.h"
#include "../../Vulkan/Data/BufferData.h"
#include "../../VulkanContext.h"
#include "../../Utils/GLTFReader.h"
#include "../../Vulkan/Pipeline.h"
#include "../DeserializableObjects/PlaneVertexedRenderObject.h"
#include "../DeserializableObjects/ColoredRenderObject.h"
#include "../DeserializableObjects/TexturedRenderObject.h"
#include "../Primitives/EvenPlaneObject.h"

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

	if constexpr (std::is_base_of_v<VertexedRenderObject, T>)
	{
		vertexDataBinding = T::VertexDataType::BindingDescription();
		vertexDataAttributes = T::VertexDataType::AttributeDescriptions();
	}

	vertexShader = T::VertexShader;
	fragmentShader = T::FragmentShader;
	pipeline = std::make_unique<Pipeline>(vulkanContext, *this);
}

template <class T>
std::weak_ptr<Shared<T>> Shared<T>::instance;


RenderObject::RenderObject(VulkanContext& vulkanContext)
	: vulkanContext(vulkanContext)
{
	std::span<TransformUniform> uniformSpan(&transformUniform, &transformUniform + 1);
	transformUniformBuffer = std::make_unique<BufferData>(BufferData::Create<TransformUniform>(
		vulkanContext, uniformSpan, MemoryType::Universal, vk::BufferUsageFlagBits::eUniformBuffer));
}

RenderObject::~RenderObject() = default;

void RenderObject::UpdateTransformUniformBuffer()
{
	TransformUniform temp;
	temp.model = transformUniform.model.Transpose();
	temp.view = transformUniform.view.Transpose();
	temp.frustum = transformUniform.frustum.Transpose();
	std::span<TransformUniform> uniformSpan(&temp, &temp + 1);
	transformUniformBuffer->FlushData(uniformSpan);
}

std::vector<vk::DescriptorSetLayoutBinding> RenderObject::DescriptorSetLayoutBinding()
{
	return {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll)
	};
}

void RenderObject::Accept(RenderVisitor& renderVisitor)
{
	renderVisitor.Visit(*this);
}

void RenderObject::Dispose()
{
	transformUniformBuffer->Dispose();
	descriptorSets->Dispose();
	shared.reset();
}

template Shared<PlaneVertexedRenderObject>;
template Shared<ColoredRenderObject>;
template Shared<TexturedRenderObject>;
template Shared<EvenPlaneObject>;
