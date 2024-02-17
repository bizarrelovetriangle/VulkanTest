#pragma once
#include "../Interfaces/VertexedRenderObject.h"

class VulkanContext;
struct DeserializedObject;

class SimpleVertexedRenderObject : public VertexedRenderObject
{
public:
	SimpleVertexedRenderObject(VulkanContext& vulkanContext);

public:
	inline static std::string VertexShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/vertexed.vert";
	inline static std::string FragmentShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/vertexed.frag";
};

