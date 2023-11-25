#pragma once
#include "../../Math/Vector3.hpp"
#include "../../Math/Vector2.hpp"
#include "../../Math/Matrix4.hpp"
#include "../../VulkanContext.h"
#include <memory>
#include "../../Vulkan/DeviceController.h"

class RenderVisitor;
class DescriptorSets;
struct DeserializedObject;
struct DeserializedObjectVertexData;
class BufferData;
class Pipeline;

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
	RenderObjectShared(VulkanContext& vulkanContext);
	~RenderObjectShared();

	vk::DescriptorSetLayout descriptorSetLayout;
	vk::VertexInputBindingDescription vertexDataBinding;
	std::vector<vk::VertexInputAttributeDescription> vertexDataAttributes;
	std::string vertexShader;
	std::string fragmentShader;
	std::unique_ptr<Pipeline> pipeline;

private:
	VulkanContext& vulkanContext;
};

template <class T>
class Shared : public RenderObjectShared
{
public:
	static std::shared_ptr<Shared<T>> getInstance(VulkanContext& vulkanContext);
	Shared(VulkanContext& vulkanContext);

private:
	static std::weak_ptr<Shared<T>> instance;
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
	std::unique_ptr<BufferData> uniformBuffer;
	std::unique_ptr<DescriptorSets> descriptorSets;
	std::shared_ptr<RenderObjectShared> shared;
};

