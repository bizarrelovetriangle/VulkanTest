#pragma once
#include "RenderObject.h"

struct DeserializedObjectVertexData;

class VertexData
{
public:
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
	VertexedRenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject);
	~VertexedRenderObject();
	virtual void Accept(RenderVisitor& renderVisitor) const;
	virtual void Dispose() override;

public:
	std::unique_ptr<BufferData> vertexBuffer;
};

