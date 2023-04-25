#pragma once
#include "../Math/Vector3.hpp"
#include "../Math/Vector2.hpp"
#include "../Math/Matrix4.hpp"
#include "../VulkanContext.h"
#include <optional>
#include <vulkan/vulkan.hpp>
#include "../Vulkan/DeviceController.h"

class RenderVisitor;
class DescriptorSets;
struct DeserializedObject;
struct DeserializedObjectVertexData;
template <class T>
class BufferMemory;

class RenderObjectPushConstantRange
{
public:
	Matrix4 model;
	Matrix4 world;
};

class RenderObjectUniform
{
public:
	alignas(16) Vector4f baseColor;
	alignas(4) bool hasTexture = false;
	alignas(4) bool hasColors = false;
};

class RenderObjectShared
{
public:
	vk::DescriptorSetLayout descriptorSetLayout;
	vk::VertexInputBindingDescription vertexDataBinding;
	std::vector<vk::VertexInputAttributeDescription> vertexDataAttributes;
	std::string vertexShader;
	std::string fragmentShader;
};

template <class T>
class Shared : public RenderObjectShared
{
public:
	Shared(VulkanContext& vulkanContext)
		: vulkanContext(vulkanContext)
	{
		auto& device = vulkanContext.deviceController->device;
		auto bindings = T::DescriptorSetLayoutBinding();
		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreate({}, bindings);
		descriptorSetLayout = device.createDescriptorSetLayout(descriptorSetLayoutCreate);
		vertexDataBinding = T::VertexDataType::BindingDescription();
		vertexDataAttributes = T::VertexDataType::AttributeDescriptions();
		vertexShader = T::VertexShader;
		fragmentShader = T::FragmentShader;
	}

	void Dispose()
	{
		auto& device = vulkanContext.deviceController->device;
		device.destroyDescriptorSetLayout(descriptorSetLayout);
	}

private:
	VulkanContext& vulkanContext;
};

class RenderObject
{
public:
	RenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject);
	static std::vector<vk::DescriptorSetLayoutBinding> DescriptorSetLayoutBinding();
	~RenderObject();
	virtual void Accept(RenderVisitor& renderVisitor) const;
	virtual void Dispose();

public:
	std::string name;
	Matrix4 model;
	RenderObjectUniform uniform;
	std::unique_ptr<BufferMemory<RenderObjectUniform>> uniformBuffer;
	std::unique_ptr<DescriptorSets> descriptorSets;
};

