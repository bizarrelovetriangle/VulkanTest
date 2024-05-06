#include "Renderer.h"
#include "../../RenderVisitor.h"
#include "../../Vulkan/DescriptorSets.h"
#include "../../Utils/GLTFReader.h"
#include "../../Vulkan/Data/BufferData.h"
#include "../../VulkanContext.h"
#include "../../Utils/GLTFReader.h"
#include "../../Vulkan/Pipeline.h"
#include "../SimpleVertexedRenderer.h"
#include "../ColoredRenderer.h"
#include "../TexturedRenderer.h"
#include "../PlaneRenderer.h"
#include "../../Objects/Primitives/BoundingBoxObject.h"
#include "../Interfaces/VertexedRenderer.h"
#include "../LinedRenderer.h"
#include "../../Camera.h"
#include "../../Vulkan/DeviceController.h"

RendererShared::RendererShared(VulkanContext& vulkanContext)
	: vulkanContext(vulkanContext)
{
}

RendererShared::~RendererShared()
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
	: RendererShared(vulkanContext)
{
	auto& device = vulkanContext.deviceController->device;
	auto bindings = T::DescriptorSetLayoutBinding();
	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreate({}, bindings);
	descriptorSetLayout = device.createDescriptorSetLayout(descriptorSetLayoutCreate);

	if constexpr (std::is_base_of_v<VertexedRenderer, T>)
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


Renderer::Renderer(VulkanContext& vulkanContext)
	: vulkanContext(vulkanContext)
{
	transformUniformBuffer = BufferData::Create<TransformUniform>(
		vulkanContext, transformUniform, MemoryType::Universal, vk::BufferUsageFlagBits::eUniformBuffer);
	propertiesUniformBuffer = BufferData::Create<PropertiesUniform>(
		vulkanContext, propertiesUniform, MemoryType::Universal, vk::BufferUsageFlagBits::eUniformBuffer);
}

Renderer::~Renderer() = default;

void Renderer::UpdateTransformUniformBuffer()
{
	transformUniformBuffer->FlushData(transformUniform);
}

void Renderer::UpdatePropertiesUniformBuffer()
{
	propertiesUniformBuffer->FlushData(propertiesUniform);
}

std::vector<vk::DescriptorSetLayoutBinding> Renderer::DescriptorSetLayoutBinding()
{
	return {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll)
	};
}

void Renderer::Accept(RenderVisitor& renderVisitor)
{
	renderVisitor.Visit(*this);
}

void Renderer::Dispose()
{
	transformUniformBuffer->Dispose();
	propertiesUniformBuffer->Dispose();
	descriptorSets->Dispose();
	shared.reset();
}

template Shared<SimpleVertexedRenderer>;
template Shared<LinedRenderer>;
template Shared<ColoredRenderer>;
template Shared<TexturedRenderer>;
template Shared<PlaneRenderer>;
