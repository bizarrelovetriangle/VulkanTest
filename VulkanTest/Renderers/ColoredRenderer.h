#pragma once
#include "../Math/Vector3.h"
#include "../Math/Vector2.h"
#include "../Math/Matrix4.h"
#include "Interfaces/VertexedRenderer.h"

class VulcanContext;
struct DeserializedObject;
struct DeserializedObjectVertexData;
class MeshModel;

class ColoredVertexData : public VertexData
{
public:
	static vk::VertexInputBindingDescription BindingDescription();
	static std::vector<vk::VertexInputAttributeDescription> AttributeDescriptions();

public:
	Vector4f color;
};

class ColoredRenderer : public VertexedRenderer
{
public:
	using VertexDataType = ColoredVertexData;
	ColoredRenderer(VulkanContext& vulkanContext, const std::vector<Vector4f>& colors, bool faceColoring = false);
	virtual void UpdateVertexBuffer(const MeshModel& mesh) override;

public:
	inline static std::string VertexShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/colored.vert";
	inline static std::string FragmentShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/colored.frag";

private:
	std::vector<Vector4f> colors;
	bool faceColoring;
};

