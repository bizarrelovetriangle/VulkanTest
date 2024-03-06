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
	Vector4f color;
};


class LinedRenderObject : public VertexedRenderObject
{
public:
	using VertexDataType = LinedVertexData;
	LinedRenderObject(VulkanContext& vulkanContext, std::vector<Vector4f> colors = {});
	virtual void UpdateVertexBuffer(const MeshModel& mesh) override;

	std::vector<Vector4f> colors;
public:
	inline static std::string VertexShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/lined.vert";
	inline static std::string FragmentShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/lined.frag";
};

