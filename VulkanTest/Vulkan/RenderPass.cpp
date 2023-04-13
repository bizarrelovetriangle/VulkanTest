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

	vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);

	vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, {}, colorAttachmentRef);

	vk::SubpassDependency dependency(
		VK_SUBPASS_EXTERNAL, 0,
		vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite);

	vk::RenderPassCreateInfo renderPassInfo({}, colorAttachment, subpass, dependency);
	renderPass = device.createRenderPass(renderPassInfo);
}

void RenderPass::Dispose()
{
	device.destroyRenderPass(renderPass);
}
