#pragma once
#include "RenderObject.h"

struct DeserializedObjectVertexData;

class VertexData
{
public:
	VertexData(const Vector3f& position, const Vector3f& normal);
	VertexData(const DeserializedObjectVertexData& deserializingObjectVertexData);
	static vk::VertexInputBindingDescription BindingDescription();
	static std::vector<vk::VertexInputAttributeDescription> AttributeDescriptions();

public:
	Vector3f position;
	Vector3f normal;
};

class VertexedRenderObject : public RenderObject
{
public:
	using VertexDataType = VertexData;
	VertexedRenderObject(VulkanContext& vulkanContext);
	~VertexedRenderObject();
	virtual void Accept(RenderVisitor& renderVisitor);
	virtual void Dispose() override;

	inline static std::string VertexShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/vertexed.vert";
	inline static std::string FragmentShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/vertexed.frag";

public:
	std::unique_ptr<BufferData> vertexBuffer;
};

