#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Pipeline.h"
#include <vector>
#include <string>
#include <fstream>
#include "SwapChain.h"
#include "VertexData.h"
#include "../Utils/ObjReader.hpp"

Pipeline::Pipeline(const vk::Device& device, const vk::RenderPass& renderPass, std::shared_ptr<SwapChain> swapChain)
	: device(device), renderPass(renderPass), swapChain(swapChain)
{
	vertShaderModule = CreateShaderModule("E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/spir-v/triangle.vert.spv");
	fragShaderModule = CreateShaderModule("E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/spir-v/triangle.frag.spv");

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo(
		{}, vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main");
	vk::PipelineShaderStageCreateInfo fragShaderStageInfo(
		{}, vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main");

	shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

	vk::PushConstantRange pushConstant(vk::ShaderStageFlagBits::eVertex, 0, sizeof(RenderObjectPushConstantRange));
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, {}, pushConstant);
	pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);

	auto binding = RenderObjectVertexData::BindingDescription();
	auto attributes = RenderObjectVertexData::AttributeDescriptions();
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo({}, binding, attributes);;
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

	std::vector<vk::DynamicState> dynamicStates{vk::DynamicState::eViewport, vk::DynamicState::eScissor};
	vk::PipelineViewportStateCreateInfo viewportState({}, 1, nullptr, 1, nullptr);
	vk::PipelineDynamicStateCreateInfo dynamicState({}, dynamicStates);

	vk::PipelineRasterizationStateCreateInfo rasterizer(
		{}, VK_FALSE, VK_FALSE,
		vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise,
		VK_FALSE, 0.0f, 0.0f, 0.0f,
		1.);

	vk::PipelineMultisampleStateCreateInfo multisampling(
		{}, vk::SampleCountFlagBits::e1,
		VK_FALSE, 1.0f, nullptr, VK_FALSE, VK_FALSE);

	vk::PipelineColorBlendAttachmentState colorBlendAttachment(
		VK_FALSE,
		vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
		vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
		vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
		vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
	);

	vk::PipelineColorBlendStateCreateInfo colorBlending({}, VK_FALSE, vk::LogicOp::eCopy, colorBlendAttachment);

	vk::GraphicsPipelineCreateInfo pipelineInfo(
		{}, shaderStages, &vertexInputInfo, &inputAssembly, nullptr,
		&viewportState, &rasterizer, &multisampling,
		nullptr, &colorBlending, &dynamicState,
		pipelineLayout, renderPass, 0);

	auto res = device.createGraphicsPipeline(VK_NULL_HANDLE, pipelineInfo);

	if (res.result != vk::Result::eSuccess){
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	graphicsPipeline = res.value;
}

void Pipeline::Dispose()
{
	device.destroyPipeline(graphicsPipeline);
	device.destroyPipelineLayout(pipelineLayout);

	device.destroyShaderModule(fragShaderModule);
	device.destroyShaderModule(vertShaderModule);
}

vk::Viewport Pipeline::CreateViewport()
{
	vk::Viewport viewport(
		0., 0.,
		(float)swapChain->swapChainExtent.width, (float)swapChain->swapChainExtent.height,
		0., 1.);
	return viewport;
}

vk::Rect2D Pipeline::CreateScissors()
{
	vk::Rect2D scissors({}, swapChain->swapChainExtent);
	return scissors;
}

vk::ShaderModule Pipeline::CreateShaderModule(std::string path) {
	std::vector<char> code;
	std::ifstream file(path, std::ios::ate | std::ios::binary);
	if (!file) throw std::runtime_error("file is not found");
	size_t fileSize = (size_t) file.tellg();
	code.resize(fileSize);
	file.seekg(0);
	file.read(code.data(), fileSize);

	std::vector<uint32_t> packedCode(code.size() / sizeof(uint32_t));
	std::memcpy(packedCode.data(), code.data(), code.size());
	vk::ShaderModuleCreateInfo createInfo({}, packedCode);
	return device.createShaderModule(createInfo);
}