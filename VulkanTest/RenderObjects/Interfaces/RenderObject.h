#pragma once
#include "../../Math/Vector3.h"
#include "../../Math/Vector2.h"
#include "../../Math/Matrix4.h"
#include "../../VulkanContext.h"
#include <memory>
#include "../../Vulkan/DeviceController.h"

class RenderVisitor;
class DescriptorSets;
struct DeserializedObject;
struct DeserializedObjectVertexData;
class BufferData;
class Pipeline;

class TransformUniform
{
public:
	alignas(16) Matrix4 model;
	alignas(16) Matrix4 view;
	alignas(16) Matrix4 frustum;
};

class PropertiesUniform
{
public:
	alignas(16) Vector4f baseColor = Vector4f(0., 0.2, 0.2, 1.);
	alignas(4) bool hasTexture = false;
	alignas(4) bool hasColors = false;
};

class RenderObjectShared
{
public:
	RenderObjectShared(VulkanContext& vulkanContext);
	~RenderObjectShared();

	vk::DescriptorSetLayout descriptorSetLayout;
	std::vector<vk::VertexInputBindingDescription> vertexDataBindings;
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
	static std::shared_ptr<Shared<T>> getInstance(VulkanContext& vulkanContext, bool lined = false);
	Shared(VulkanContext& vulkanContext, bool lined);

private:
	static std::weak_ptr<Shared<T>> instance;
};


class RenderObject
{
public:
	RenderObject(VulkanContext& vulkanContext);
	virtual ~RenderObject();

	void UpdateTransformUniformBuffer();
	void UpdatePropertiesUniformBuffer();
	static std::vector<vk::DescriptorSetLayoutBinding> DescriptorSetLayoutBinding();
	virtual void Accept(RenderVisitor& renderVisitor, const Camera& camera);
	virtual void Dispose();

public:
	TransformUniform transformUniform;
	std::unique_ptr<BufferData> transformUniformBuffer;
	PropertiesUniform propertiesUniform;
	std::unique_ptr<BufferData> propertiesUniformBuffer;

	std::unique_ptr<DescriptorSets> descriptorSets;
	std::shared_ptr<RenderObjectShared> shared;

protected:
	VulkanContext& vulkanContext;
};

