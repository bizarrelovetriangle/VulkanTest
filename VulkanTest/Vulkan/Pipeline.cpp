#include "Pipeline.h"
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include "SwapChain.h"
#include "../Primitives/RenderObject.h"
#include "Memory/ImageMemory.h"
#include "../VulkanContext.h"
#include "../Utils/ShaderCompiler.h"

Pipeline::Pipeline(VulkanContext& vulkanContext,
	const vk::Device& device, const vk::RenderPass& renderPass, std::shared_ptr<SwapChain> swapChain)
	: vulkanContext(vulkanContext), device(device), renderPass(renderPass), swapChain(swapChain)
{
	auto vertexSpirv = ShaderCompiler::CompileShader(
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/triangle.vert",
		vk::ShaderStageFlagBits::eVertex, false);
	vertShaderModule = device.createShaderModule(vk::ShaderModuleCreateInfo({}, vertexSpirv));

	auto fragmentSpirv = ShaderCompiler::CompileShader(
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/triangle.frag",
		vk::ShaderStageFlagBits::eFragment, false);
	fragShaderModule = device.createShaderModule(vk::ShaderModuleCreateInfo({}, fragmentSpirv));

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo(
		{}, vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main");
	vk::PipelineShaderStageCreateInfo fragShaderStageInfo(
		{}, vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main");

	shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

	{
		auto imageInfo = ImageMemory::LoadImage("E:/Images/testImage.jpeg");

		image = std::make_unique<ImageMemory>(vulkanContext,
			imageInfo.first, vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eSampled, vk::ImageAspectFlagBits::eColor,
			MemoryType::DeviceLocal);
		image->FlushData(imageInfo.second);
		image->TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	}

	CreateDescriptorSetLayout();
	vk::PushConstantRange pushConstant(vk::ShaderStageFlagBits::eVertex, 0, sizeof(RenderObjectPushConstantRange));
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, descriptorSetLayout, pushConstant);
	pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);

	auto binding = RenderObjectVertexData::BindingDescription();
	auto attributes = RenderObjectVertexData::AttributeDescriptions();
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo({}, binding, attributes);;
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
		pipelineLayout, renderPass, 0);

	auto res = device.createGraphicsPipeline(nullptr, pipelineInfo);

	if (res.result != vk::Result::eSuccess){
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	graphicsPipeline = res.value;
}

void Pipeline::Dispose()
{
	image->Dispose();

	device.destroyDescriptorSetLayout(descriptorSetLayout);

	device.destroyPipeline(graphicsPipeline);
	device.destroyPipelineLayout(pipelineLayout);

	device.destroyShaderModule(fragShaderModule);
	device.destroyShaderModule(vertShaderModule);
}

void Pipeline::CreateDescriptorSetLayout()
{
	std::vector<vk::DescriptorSetLayoutBinding> bindings
	{
		vk::DescriptorSetLayoutBinding(
			0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll),
		vk::DescriptorSetLayoutBinding(
			1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment)
	};
	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreate({}, bindings);
	descriptorSetLayout = device.createDescriptorSetLayout(descriptorSetLayoutCreate);
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
