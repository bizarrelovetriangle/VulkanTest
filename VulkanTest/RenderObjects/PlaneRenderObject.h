#pragma once
#include "Interfaces/VertexedRenderObject.h"

class RenderVisitor;
class BufferData;

class PlaneObjectUniform
{
public:
	alignas(16) Vector4f color;
	alignas(4) bool grided = true;
};

class PlaneRenderObject : public RenderObject
{
public:
	PlaneRenderObject(VulkanContext& vulkanContext);
	~PlaneRenderObject();
	void UpdatePlaneUniformBuffer();
	virtual void Dispose() override;
	static std::vector<vk::DescriptorSetLayoutBinding> DescriptorSetLayoutBinding();
	virtual void Accept(RenderVisitor& renderVisitor) override;

public:
	inline static std::string VertexShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/Primitives/EvenPlane.vert";
	inline static std::string FragmentShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/Primitives/EvenPlane.frag";

	PlaneObjectUniform evenPlaneObjectUniform;
	std::unique_ptr<BufferData> evenPlaneObjectUniformBuffer;
};
