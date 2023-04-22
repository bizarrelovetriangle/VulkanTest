#pragma once
#include "../Math/Vector3.hpp"
#include "../Math/Vector2.hpp"
#include "../Math/Matrix4.hpp"
#include "../Vulkan/Memory/BufferMemory.h"
#include <optional>
#include <vulkan/vulkan.hpp>

class RenderVisitor;
class ImageMemory;
class DescriptorSets;
class Pipeline;

class RenderObjectPushConstantRange
{
public:
	Matrix4 model;
	Matrix4 world;
};

class RenderObjectVertexData
{
public:
	Vector3f position;
	Vector3f normal;
	Vector2f textureCoord;
	Vector4f color;

	static vk::VertexInputBindingDescription BindingDescription()
	{
		return vk::VertexInputBindingDescription(0, sizeof(RenderObjectVertexData), vk::VertexInputRate::eVertex);
	}

	static std::vector<vk::VertexInputAttributeDescription> AttributeDescriptions()
	{
		vk::VertexInputAttributeDescription positionDescription(
			0, 0, vk::Format::eR32G32B32Sfloat, offsetof(RenderObjectVertexData, position));
		vk::VertexInputAttributeDescription normalDescription(
			1, 0, vk::Format::eR32G32B32Sfloat, offsetof(RenderObjectVertexData, normal));
		vk::VertexInputAttributeDescription textureCoordDescription(
			2, 0, vk::Format::eR32G32Sfloat, offsetof(RenderObjectVertexData, textureCoord));
		vk::VertexInputAttributeDescription colorDescription(
			3, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(RenderObjectVertexData, color));

		return { positionDescription, normalDescription, textureCoordDescription, colorDescription };
	}
};

class RenderObjectUniform
{
public:
	alignas(16) Vector4f baseColor;
	alignas(4) bool hasTexture = false;
	alignas(4) bool hasColors = false;
};

class RenderObject
{
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

	RenderObject();
	virtual void Accept(RenderVisitor& renderVisitor) const;
	void Dispose();
};

