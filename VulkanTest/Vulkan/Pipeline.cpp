#include "Pipeline.h"
#include <vector>
#include <string>
#include <fstream>
#include "SwapChain.h"
#include "../Primitives/RenderObject.h"
#include "Memory/ImageMemory.h"
#include "../VulkanContext.h"

Pipeline::Pipeline(VulkanContext& vulkanContext,
	const vk::Device& device, const vk::RenderPass& renderPass, std::shared_ptr<SwapChain> swapChain)
	: vulkanContext(vulkanContext), device(device), renderPass(renderPass), swapChain(swapChain)
{
	vertShaderModule = CreateShaderModule("E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/spir-v/triangle.vert.spv");
	fragShaderModule = CreateShaderModule("E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/spir-v/triangle.frag.spv");

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo(
		{}, vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main");
	vk::PipelineShaderStageCreateInfo fragShaderStageInfo(
		{}, vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main");

	shaderStages = { vertShaderStageInfo, fragShaderStageInfo };


	{
		auto imageInfo = ImageMemory::LoadImage("E:/Images/testImage.jpeg");

		image = std::make_unique<ImageMemory>(vulkanContext,
			imageInfo.first, vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eSampled,
			MemoryType::DeviceLocal);

		image->FlushData(imageInfo.second);
	}

	CreateDescriptorSetLayout();
	vk::PushConstantRange pushConstant(vk::ShaderStageFlagBits::eVertex, 0, sizeof(RenderObjectPushConstantRange));
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, descriptorSetLayout, pushConstant);
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
	image->Dispose();

	device.destroyDescriptorSetLayout(descriptorSetLayout);
	device.destroyDescriptorPool(descriptorPool);

	device.destroyPipeline(graphicsPipeline);
	device.destroyPipelineLayout(pipelineLayout);

	device.destroyShaderModule(fragShaderModule);
	device.destroyShaderModule(vertShaderModule);
}

void Pipeline::CreateDescriptorSetLayout()
{
	vk::DescriptorSetLayoutBinding binding(
		0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreate({}, binding);
	descriptorSetLayout = device.createDescriptorSetLayout(descriptorSetLayoutCreate);

	vk::DescriptorPoolSize descriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, swapChain->frameCount);
	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo({}, swapChain->frameCount, descriptorPoolSize);
	descriptorPool = device.createDescriptorPool(descriptorPoolCreateInfo);

	std::vector<vk::DescriptorSetLayout> layouts(swapChain->frameCount, descriptorSetLayout);
	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(descriptorPool, layouts);
	descriptorSets = device.allocateDescriptorSets(descriptorSetAllocateInfo);

	for (size_t i = 0; i < swapChain->frameCount; ++i)
	{
		vk::DescriptorImageInfo descriptorImageInfo(
			image->sampler, image->imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::WriteDescriptorSet writeDescriptorSet(
			descriptorSets[i], 0, 0, vk::DescriptorType::eCombinedImageSampler, descriptorImageInfo, {}, {});
		device.updateDescriptorSets(writeDescriptorSet, {});
	}
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