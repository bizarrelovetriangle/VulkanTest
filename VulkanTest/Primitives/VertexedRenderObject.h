#pragma once
#include "RenderObject.h"

struct DeserializedObjectVertexData;
template <class T>
class BufferMemory;

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

template <class T = VertexData>
class VertexedRenderObject : public RenderObject
{
public:
	using VertexDataType = T;
	VertexedRenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject);
	~VertexedRenderObject();
	virtual void Accept(RenderVisitor& renderVisitor) const;
	virtual void Dispose() override;

public:
	inline static std::string VertexShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/vertexed.vert";
	inline static std::string FragmentShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/vertexed.frag";

	std::vector<T> vertexData;
	std::unique_ptr<BufferMemory<T>> vertexBuffer;
};

