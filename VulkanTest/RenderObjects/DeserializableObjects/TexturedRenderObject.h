#pragma once
#include "../Interfaces/VertexedRenderObject.h"

class VulcanContext;
struct DeserializedObject;
struct DeserializedObjectVertexData;
class ImageData;

class TexturedVertexData : public VertexData
{
public:
	TexturedVertexData(const DeserializedObjectVertexData& deserializingObjectVertexData);
	static vk::VertexInputBindingDescription BindingDescription();
	static std::vector<vk::VertexInputAttributeDescription> AttributeDescriptions();

public:
	Vector2f textureCoord;
};

class TexturedRenderObject : public VertexedRenderObject
{
public:
	using VertexDataType = TexturedVertexData;

	TexturedRenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject);
	~TexturedRenderObject();
	static std::vector<vk::DescriptorSetLayoutBinding> DescriptorSetLayoutBinding();
	void Dispose();

public:
	inline static std::string VertexShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/textured.vert";
	inline static std::string FragmentShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/textured.frag";

	std::pair<Vector2u, std::vector<std::byte>> textureData;
	std::unique_ptr<ImageData> textureBuffer;

private:
	std::vector<TexturedVertexData> vertexData;
};

