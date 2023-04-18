#include "RenderPass.h"
#include <vector>
#include <string>
#include <fstream>

RenderPass::RenderPass(const vk::Device& device, vk::Format swapChainImageFormat)
	: device(device), swapChainImageFormat(swapChainImageFormat)
{
	vk::AttachmentDescription colorAttachment(
		{}, swapChainImageFormat, vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
		vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);

	vk::AttachmentDescription depthAttachment(
		{}, vk::Format::eD32Sfloat, vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare,
		vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);
	vk::AttachmentReference depthAttachmentRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, {}, colorAttachmentRef, {}, &depthAttachmentRef);

	vk::SubpassDependency dependency(
		VK_SUBPASS_EXTERNAL, 0,
		vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
		vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite );

	auto renderPassAttachments = { colorAttachment, depthAttachment };
	vk::RenderPassCreateInfo renderPassInfo({}, renderPassAttachments, subpass, dependency);
	renderPass = device.createRenderPass(renderPassInfo);
}

void RenderPass::Dispose()
{
	device.destroyRenderPass(renderPass);
}
