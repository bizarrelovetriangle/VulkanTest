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
	std::vector<vk::DynamicState> dynamicStates = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
	};

	auto dynamicStagesInfo = DescribeDinamicStages(dynamicStates);

	CreateShaderStages();
	pipelineLayout = CreatePipelineLayout();

	auto binding = ObjVertexData::BindingDescription();
	auto attributes = ObjVertexData::AttributeDescriptions();
	auto vertexInputInfo = CreateVertexInputInfo(binding, attributes);
	auto inputAssemblyInfo = CreateInputAssemblyInfo();
	auto viewportStateState = CreateViewportState();
	auto rasterizer = CreateRasterizer();
	auto multisampling = CreateMultisampling();
	auto colorBlendAttachment = CreateColorBlendAttachment();
	auto colorBlendState = CreateColorBlendAttachment(colorBlendAttachment);

	vk::GraphicsPipelineCreateInfo pipelineInfo
	{
		.stageCount = (uint32_t) shaderStages.size(),
		.pStages = shaderStages.data(),
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &inputAssemblyInfo,
		.pViewportState = &viewportStateState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = nullptr, // Optional
		.pColorBlendState = &colorBlendState,
		.pDynamicState = &dynamicStagesInfo,
		.layout = pipelineLayout,

		.renderPass = renderPass,
		.subpass = 0
	};

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
	vk::Viewport viewport
	{
		.x = 0.0f,
		.y = 0.0f,
		.width = (float)swapChain->swapChainExtent.width,
		.height = (float)swapChain->swapChainExtent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	return viewport;
}

vk::Rect2D Pipeline::CreateScissors()
{
	vk::Rect2D scissors
	{
		.offset = { 0, 0 },
		.extent = swapChain->swapChainExtent
	};

	return scissors;
}

void Pipeline::CreateShaderStages() {
	vertShaderModule = CreateShaderModule("E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/spir-v/triangle.vert.spv");
	fragShaderModule = CreateShaderModule("E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/spir-v/triangle.frag.spv");

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo
	{
		.stage = vk::ShaderStageFlagBits::eVertex,
		.module = vertShaderModule,
		.pName = "main"
	};

	vk::PipelineShaderStageCreateInfo fragShaderStageInfo
	{
		.stage = vk::ShaderStageFlagBits::eFragment,
		.module = fragShaderModule,
		.pName = "main"
	};

	shaderStages = { vertShaderStageInfo, fragShaderStageInfo };
}

vk::PipelineLayout Pipeline::CreatePipelineLayout() {
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo
	{
		.setLayoutCount = 0, // Optional
		.pSetLayouts = nullptr, // Optional
		.pushConstantRangeCount = 0, // Optional
		.pPushConstantRanges = nullptr // Optional
	};

	return device.createPipelineLayout(pipelineLayoutInfo);
}

vk::PipelineVertexInputStateCreateInfo Pipeline::CreateVertexInputInfo(
	vk::VertexInputBindingDescription binding, std::vector<vk::VertexInputAttributeDescription>& attributes)
{
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo
	{
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &binding,
		.vertexAttributeDescriptionCount = (uint32_t) attributes.size(),
		.pVertexAttributeDescriptions = attributes.data()
	};
	return vertexInputInfo;
}

vk::PipelineInputAssemblyStateCreateInfo Pipeline::CreateInputAssemblyInfo() {
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly
	{
		.topology = vk::PrimitiveTopology::eTriangleList,
		.primitiveRestartEnable = VK_FALSE
	};
	return inputAssembly;
}

vk::PipelineDynamicStateCreateInfo Pipeline::DescribeDinamicStages(std::vector<vk::DynamicState>& dynamicStates) {
	vk::PipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();
	return dynamicState;
}

vk::PipelineViewportStateCreateInfo Pipeline::CreateViewportState() {
	vk::PipelineViewportStateCreateInfo viewportState
	{
		.viewportCount = 1,
		.scissorCount = 1
	};
	return viewportState;
}

vk::PipelineRasterizationStateCreateInfo Pipeline::CreateRasterizer() {
	vk::PipelineRasterizationStateCreateInfo rasterizer
	{
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,

		.polygonMode = vk::PolygonMode::eFill,

		.cullMode = vk::CullModeFlagBits::eBack,
		.frontFace = vk::FrontFace::eClockwise,

		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.0f, // Optional
		.depthBiasClamp = 0.0f, // Optional
		.depthBiasSlopeFactor = 0.0f, // Optional

		.lineWidth = 1.
	};
	return rasterizer;
}

vk::PipelineMultisampleStateCreateInfo Pipeline::CreateMultisampling() {
	vk::PipelineMultisampleStateCreateInfo multisampling
	{
		.rasterizationSamples = vk::SampleCountFlagBits::e1,
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 1.0f, // Optional
		.pSampleMask = nullptr, // Optional
		.alphaToCoverageEnable = VK_FALSE, // Optional
		.alphaToOneEnable = VK_FALSE // Optional
	};
	return multisampling;
}

vk::PipelineColorBlendAttachmentState Pipeline::CreateColorBlendAttachment()
{
	vk::PipelineColorBlendAttachmentState colorBlendAttachment
	{
		.blendEnable = VK_FALSE,
		.srcColorBlendFactor = vk::BlendFactor::eOne, // Optional
		.dstColorBlendFactor = vk::BlendFactor::eZero, // Optional
		.colorBlendOp = vk::BlendOp::eAdd, // Optional
		.srcAlphaBlendFactor = vk::BlendFactor::eOne, // Optional
		.dstAlphaBlendFactor = vk::BlendFactor::eZero, // Optional
		.alphaBlendOp = vk::BlendOp::eAdd, // Optional
		.colorWriteMask =
				vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
	};
	return colorBlendAttachment;
}

vk::PipelineColorBlendStateCreateInfo Pipeline::CreateColorBlendAttachment(
	vk::PipelineColorBlendAttachmentState& colorBlendAttachment)
{
	vk::PipelineColorBlendStateCreateInfo colorBlending
	{
		.logicOpEnable = VK_FALSE,
		.logicOp = vk::LogicOp::eCopy, // Optional
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment
	};
	return colorBlending;
}

vk::ShaderModule Pipeline::CreateShaderModule(std::string path) {
	std::vector<char> code;
	std::ifstream file(path, std::ios::ate | std::ios::binary);
	if (!file) throw std::runtime_error("file is not found");
	size_t fileSize = (size_t) file.tellg();
	code.resize(fileSize);
	file.seekg(0);
	file.read(code.data(), fileSize);

	vk::ShaderModuleCreateInfo createInfo
	{
		.codeSize = code.size(),
		.pCode = reinterpret_cast<const uint32_t*>(code.data())
	};

	return device.createShaderModule(createInfo);
}