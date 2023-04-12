#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "CommandBuffer.h"
#include "DeviceController.h"
#include "QueueFamilies.h"
#include <memory>
#include "Pipeline.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "VertexData.h"
#include "../Primitives/RenderObject.h"
#include "../Utils/ObjReader.hpp"
#include "../RenderVisitor.h";

CommandBuffer::CommandBuffer(const vk::Device& device, std::shared_ptr<QueueFamilies> queueFamilies,
	std::shared_ptr<Pipeline> pipeline, std::shared_ptr<SwapChain> swapChain,
	std::shared_ptr<RenderPass> renderPass)
	: device(device), queueFamilies(queueFamilies), pipeline(pipeline), swapChain(swapChain), renderPass(renderPass)
{
	CreateCommandPool();
	CreateCommandBuffer();
}

void CommandBuffer::Dispose()
{
	device.destroyCommandPool(commandPool);
}

void CommandBuffer::RecordCommandBuffer(int imageIndex,
	const std::vector<std::unique_ptr<RenderObject>>& renderObjects)
{
	vk::CommandBufferBeginInfo beginInfo;
	commandBuffer.begin(beginInfo);

	{
		vk::Rect2D renderArea({ 0, 0 }, swapChain->swapChainExtent);

		vk::ClearColorValue clearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
		vk::ClearValue clearColor(clearColorValue);

		vk::RenderPassBeginInfo renderPassInfo(
			renderPass->renderPass, swapChain->swapChainFramebuffers[imageIndex],
			renderArea, clearColor);

		commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

		RenderVisitor renderVisitor(*this, *pipeline);

		for (auto& renderObject : renderObjects)
		{
			renderObject->Accept(renderVisitor);
		}

		commandBuffer.endRenderPass();
	}

	commandBuffer.end();
}

void CommandBuffer::CommandBuffer::Reset() {
	commandBuffer.reset();
}

void CommandBuffer::CreateCommandPool()
{
	auto graphicQueueFamily = std::find_if(std::begin(queueFamilies->queueFamilies), std::end(queueFamilies->queueFamilies),
		[](auto& family) { return family.flags.contains(vk::QueueFlagBits::eGraphics) && family.presentSupport; });

	vk::CommandPoolCreateInfo poolInfo(
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer, graphicQueueFamily->index);

	commandPool = device.createCommandPool(poolInfo);
}

void CommandBuffer::CreateCommandBuffer()
{
	vk::CommandBufferAllocateInfo allocInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1);
	commandBuffer = device.allocateCommandBuffers(allocInfo).front();
}
