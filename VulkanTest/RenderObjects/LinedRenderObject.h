#pragma once
#include "Interfaces/VertexedRenderObject.h"

class VulkanContext;
struct DeserializedObject;

class LinedVertexData
{
public:
	static vk::VertexInputBindingDescription BindingDescription();
	static std::vector<vk::VertexInputAttributeDescription> AttributeDescriptions();

public:
	Vector3f position;
};


class LinedRenderObject : public VertexedRenderObject
{
public:
	using VertexDataType = LinedVertexData;
	LinedRenderObject(VulkanContext& vulkanContext);
	virtual void UpdateVertexBuffer(const MeshModel& mesh) override;

public:
	inline static std::string VertexShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/lined.vert";
	inline static std::string FragmentShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/lined.frag";
};

