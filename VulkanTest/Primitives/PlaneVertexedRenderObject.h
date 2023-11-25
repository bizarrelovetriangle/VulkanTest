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

class PlaneVertexedRenderObject : public VertexedRenderObject
{
public:
	using VertexDataType = VertexData;

	PlaneVertexedRenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject);
	~PlaneVertexedRenderObject();
	virtual void Accept(RenderVisitor& renderVisitor) const;

public:
	inline static std::string VertexShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/vertexed.vert";
	inline static std::string FragmentShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/vertexed.frag";

private:
	std::vector<VertexData> vertexData;
};

