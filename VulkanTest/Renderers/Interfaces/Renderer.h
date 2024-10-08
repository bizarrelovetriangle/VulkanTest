#pragma once
#include "../../Math/Vector3.h"
#include "../../Math/Vector2.h"
#include "../../Math/Matrix4.h"
#include <memory>

class RenderVisitor;
class DescriptorSets;
struct DeserializedObject;
struct DeserializedObjectVertexData;
class BufferData;
class Pipeline;
class VulkanContext;
class Camera;

class TransformUniform
{
public:
	alignas(16) Matrix4 modelToWorld;
};

class PropertiesUniform
{
public:
	alignas(16) Vector4f baseColor = Vector4f(0., 0.2, 0.2, 1.);
	alignas(4) bool hasTexture = false;
	alignas(4) bool hasColors = false;
};

class RendererShared
{
public:
	RendererShared(VulkanContext& vulkanContext);
	~RendererShared();

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
class Shared : public RendererShared
{
public:
	static std::shared_ptr<Shared<T>> getInstance(VulkanContext& vulkanContext, bool lined = false);
	Shared(VulkanContext& vulkanContext, bool lined);

private:
	static std::weak_ptr<Shared<T>> instance;
};


class Renderer
{
public:
	Renderer(VulkanContext& vulkanContext);
	virtual ~Renderer();

	void UpdateTransformUniformBuffer();
	void UpdatePropertiesUniformBuffer();
	static std::vector<vk::DescriptorSetLayoutBinding> DescriptorSetLayoutBinding();
	virtual void Accept(RenderVisitor& renderVisitor);
	virtual void Dispose();

public:
	TransformUniform transformUniform;
	std::unique_ptr<BufferData> transformUniformBuffer;
	PropertiesUniform propertiesUniform;
	std::unique_ptr<BufferData> propertiesUniformBuffer;

	std::unique_ptr<DescriptorSets> descriptorSets;
	std::shared_ptr<RendererShared> shared;

protected:
	VulkanContext& vulkanContext;
};

