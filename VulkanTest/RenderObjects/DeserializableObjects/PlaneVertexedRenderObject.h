#pragma once
#include "../Interfaces/VertexedRenderObject.h"

class VulkanContext;
struct DeserializedObject;

class PlaneVertexedRenderObject : public VertexedRenderObject
{
public:
	using VertexDataType = VertexData;
	PlaneVertexedRenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject);

public:
	inline static std::string VertexShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/vertexed.vert";
	inline static std::string FragmentShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/vertexed.frag";

private:
	std::vector<VertexData> vertexData;
};

