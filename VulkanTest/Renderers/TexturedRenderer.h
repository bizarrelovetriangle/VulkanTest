#pragma once
#include "Interfaces/VertexedRenderer.h"

class VulcanContext;
struct DeserializedObject;
struct DeserializedObjectVertexData;
class ImageData;
class MeshModel;

class TexturedVertexData : public VertexData
{
public:
	static vk::VertexInputBindingDescription BindingDescription();
	static std::vector<vk::VertexInputAttributeDescription> AttributeDescriptions();

public:
	Vector2f textureCoord;
};

class TexturedRenderer : public VertexedRenderer
{
public:
	using VertexDataType = TexturedVertexData;

	TexturedRenderer(VulkanContext& vulkanContext,
		std::pair<Vector2u, std::vector<std::byte>> textureData, const std::vector<Vector2f>& textureCoords);
	~TexturedRenderer();
	virtual void UpdateVertexBuffer(const MeshModel& mesh) override;
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
	std::vector<Vector2f> textureCoords;
};

