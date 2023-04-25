#pragma once
#include "../Math/Vector3.hpp"
#include "../Math/Vector2.hpp"
#include "../Math/Matrix4.hpp"
#include "../VulkanContext.h"
#include <optional>
#include <vulkan/vulkan.hpp>
#include "VertexedRenderObject.h"

class RenderVisitor;
struct DeserializedObject;
struct DeserializedObjectVertexData;
class ImageMemory;

class TexturedRenderObjectVertexData : public VertexData
{
public:
	TexturedRenderObjectVertexData(const DeserializedObjectVertexData& deserializingObjectVertexData);
	static vk::VertexInputBindingDescription BindingDescription();
	static std::vector<vk::VertexInputAttributeDescription> AttributeDescriptions();

public:
	Vector2f textureCoord;
};

class TexturedRenderObject : public VertexedRenderObject<TexturedRenderObjectVertexData>
{
public:
	TexturedRenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject);
	static std::vector<vk::DescriptorSetLayoutBinding> DescriptorSetLayoutBinding();
	~TexturedRenderObject();
	virtual void Accept(RenderVisitor& renderVisitor) const;
	void Dispose();

public:
	inline static std::string VertexShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/textured.vert";
	inline static std::string FragmentShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/textured.frag";

	std::pair<Vector2u, std::vector<std::byte>> textureData;
	std::unique_ptr<ImageMemory> textureBuffer;
};

