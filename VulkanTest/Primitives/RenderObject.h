#pragma once
#include "../Math/Vector3.hpp"
#include "../Math/Vector2.hpp"
#include "../Math/Matrix4.hpp"
#include "../VulkanContext.h"
#include <optional>
#include <vulkan/vulkan.hpp>

class RenderVisitor;
class DescriptorSets;
class Pipeline;
struct DeserializedObject;
struct DeserializedObjectVertexData;
template <class T>
class BufferMemory;
class ImageMemory;

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

class RenderObjectVertexData
{
public:
	RenderObjectVertexData(const DeserializedObjectVertexData& deserializingObjectVertexData);
	static vk::VertexInputBindingDescription BindingDescription();
	static std::vector<vk::VertexInputAttributeDescription> AttributeDescriptions();

public:
	Vector3f position;
	Vector3f normal;
	Vector2f textureCoord;
	Vector4f color;
};

class RenderObject
{
public:
	RenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject);
	~RenderObject();
	virtual void Accept(RenderVisitor& renderVisitor) const;
	void Dispose();

public:
	std::string name;
	Matrix4 model;
	std::vector<RenderObjectVertexData> vertexData;
	std::unique_ptr<BufferMemory<RenderObjectVertexData>> vertexBuffer;

	std::optional<std::pair<Vector2u, std::vector<std::byte>>> textureData;
	std::unique_ptr<ImageMemory> textureBuffer;

	RenderObjectUniform uniform;
	std::unique_ptr<BufferMemory<RenderObjectUniform>> uniformBuffer;

	std::unique_ptr<DescriptorSets> descriptorSets;
};

