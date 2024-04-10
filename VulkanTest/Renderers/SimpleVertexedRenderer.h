#pragma once
#include "Interfaces/VertexedRenderer.h"

class VulkanContext;
struct DeserializedObject;

class SimpleVertexedRenderer : public VertexedRenderer
{
public:
	SimpleVertexedRenderer(VulkanContext& vulkanContext);

public:
	inline static std::string VertexShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/vertexed.vert";
	inline static std::string FragmentShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/vertexed.frag";
};

