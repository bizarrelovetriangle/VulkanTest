#pragma once
#pragma once
#include "../Math/Vector3.h"
#include "../Math/Vector2.h"
#include "../Math/Matrix4.h"
#include "Interfaces/VertexedRenderer.h"

class VulcanContext;

class FluidRenderer : public VertexedRenderer
{
public:
	FluidRenderer(VulkanContext& vulkanContext);

	//void UpdateFluidUniformBuffer();
	//virtual void Dispose() override;
	//static std::vector<vk::DescriptorSetLayoutBinding> DescriptorSetLayoutBinding();
public:
	inline static std::string VertexShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/fluid.vert";
	inline static std::string FragmentShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/fluid.frag";

	static std::vector<vk::DescriptorSetLayoutBinding> DescriptorSetLayoutBinding();

	//FluidObjectUniform evenFluidObjectUniform;
	//std::unique_ptr<BufferData> evenFluidObjectUniformBuffer;
};

