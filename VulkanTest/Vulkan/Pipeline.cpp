#include "Pipeline.h"
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include "SwapChain.h"
#include "DeviceController.h"
#include "RenderPass.h"
#include "../RenderObjects/Interfaces/RenderObject.h"
#include "../VulkanContext.h"
#include "../Utils/ShaderCompiler.h"

Pipeline::Pipeline(VulkanContext& vulkanContext, RenderObjectShared& renderObjectShared)
	: vulkanContext(vulkanContext)
{
	auto& device = vulkanContext.deviceController->device;

	auto vertexSpirv = ShaderCompiler::CompileShader(renderObjectShared.vertexShader,
		vk::ShaderStageFlagBits::eVertex, false);
	vertShaderModule = device.createShaderModule(vk::ShaderModuleCreateInfo({}, vertexSpirv));

	auto fragmentSpirv = ShaderCompiler::CompileShader(renderObjectShared.fragmentShader,
		vk::ShaderStageFlagBits::eFragment, false);
	fragShaderModule = device.createShaderModule(vk::ShaderModuleCreateInfo({}, fragmentSpirv));

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo(
		{}, vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main");
	vk::PipelineShaderStageCreateInfo fragShaderStageInfo(
		{}, vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main");

	shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, renderObjectShared.descriptorSetLayout, {});
	pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo(
		{}, renderObjectShared.vertexDataBinding, renderObjectShared.vertexDataAttributes);;
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly({}, vk::PrimitiveTopology::eTriangleList, false);

	std::vector<vk::DynamicState> dynamicStates{vk::DynamicState::eViewport, vk::DynamicState::eScissor};
	vk::PipelineViewportStateCreateInfo viewportState({}, 1, nullptr, 1, nullptr);
	vk::PipelineDynamicStateCreateInfo dynamicState({}, dynamicStates);

	vk::PipelineRasterizationStateCreateInfo rasterizer(
		{}, false, false,
		vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eCounterClockwise,
		false, 0.0f, 0.0f, 0.0f,
		1.);

	vk::PipelineMultisampleStateCreateInfo multisampling(
		{}, vk::SampleCountFlagBits::e1,
		false, 1.0f, nullptr, false, false);

	vk::PipelineDepthStencilStateCreateInfo depthStencil(
		{}, true, true, vk::CompareOp::eLess, false, false, {}, {}, 0., 1.);

	vk::PipelineColorBlendAttachmentState colorBlendAttachment(
		false,
		vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
		vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
		vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
		vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
	);

	vk::PipelineColorBlendStateCreateInfo colorBlending({}, false, vk::LogicOp::eCopy, colorBlendAttachment);

	vk::GraphicsPipelineCreateInfo pipelineInfo(
		{}, shaderStages, &vertexInputInfo, &inputAssembly, nullptr,
		&viewportState, &rasterizer, &multisampling,
		&depthStencil, &colorBlending, &dynamicState,
		pipelineLayout, vulkanContext.renderPass->renderPass, 0);

	auto res = device.createGraphicsPipeline(nullptr, pipelineInfo);

	if (res.result != vk::Result::eSuccess){
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	graphicsPipeline = res.value;
}

void Pipeline::Dispose()
{
	auto& device = vulkanContext.deviceController->device;

	device.destroyPipeline(graphicsPipeline);
	device.destroyPipelineLayout(pipelineLayout);

	device.destroyShaderModule(fragShaderModule);
	device.destroyShaderModule(vertShaderModule);
}
