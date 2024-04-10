#pragma once
#include "Interfaces/VertexedRenderer.h"

class RenderVisitor;
class BufferData;

class PlaneObjectUniform
{
public:
	alignas(16) Vector4f color;
	alignas(4) bool grided = true;
};

class PlaneRenderer : public VertexedRenderer
{
public:
	PlaneRenderer(VulkanContext& vulkanContext);
	~PlaneRenderer();
	void UpdatePlaneUniformBuffer();
	virtual void Dispose() override;
	static std::vector<vk::DescriptorSetLayoutBinding> DescriptorSetLayoutBinding();

public:
	inline static std::string VertexShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/Primitives/EvenPlane.vert";
	inline static std::string FragmentShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/Primitives/EvenPlane.frag";

	PlaneObjectUniform evenPlaneObjectUniform;
	std::unique_ptr<BufferData> evenPlaneObjectUniformBuffer;
};
