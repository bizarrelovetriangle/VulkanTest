#pragma once
#include "../Math/Vector3.hpp"
#include "../Math/Vector2.hpp"
#include "../Math/Matrix4.hpp"
#include "../VulkanContext.h"
#include <optional>
#include <vulkan/vulkan.hpp>
#include "Interfaces/VertexedRenderObject.h"

class RenderVisitor;
class DescriptorSets;
class Pipeline;
struct DeserializedObject;
struct DeserializedObjectVertexData;

class ColoredVertexData : public VertexData
{
public:
	ColoredVertexData(const DeserializedObjectVertexData& deserializingObjectVertexData);
	static vk::VertexInputBindingDescription BindingDescription();
	static std::vector<vk::VertexInputAttributeDescription> AttributeDescriptions();

public:
	Vector4f color;
};

class ColoredRenderObject : public VertexedRenderObject
{
public:
	using VertexDataType = ColoredVertexData;

	ColoredRenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject);
	~ColoredRenderObject();
	virtual void Accept(RenderVisitor& renderVisitor) const;

public:
	inline static std::string VertexShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/colored.vert";
	inline static std::string FragmentShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/colored.frag";

private:
	std::vector<ColoredVertexData> vertexData;
};

