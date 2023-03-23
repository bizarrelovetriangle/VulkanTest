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
#include "VulkanBuffer.h"
#include "VertexData.h"
#include "../Utils/ObjReader.hpp"

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
	const VulkanBuffer<RenderObjectVertexData>& vertexBuffer)
{
	vk::CommandBufferBeginInfo beginInfo;
	commandBuffer.begin(beginInfo);

	{
		vk::Rect2D renderArea
		{
			.offset = { 0, 0 },
			.extent = swapChain->swapChainExtent,
		};

		vk::ClearColorValue clearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
		vk::ClearValue clearColor{ .color = clearColorValue };

		vk::RenderPassBeginInfo renderPassInfo
		{
			.renderPass = renderPass->renderPass,
			.framebuffer = swapChain->swapChainFramebuffers[imageIndex],
			.renderArea = renderArea,
			.clearValueCount = 1,
			.pClearValues = &clearColor
		};

		commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

		{
			commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->graphicsPipeline);
			auto viewport = pipeline->CreateViewport();
			auto scissors = pipeline->CreateScissors();
			commandBuffer.setViewport(0, 1, &viewport);
			commandBuffer.setScissor(0, 1, &scissors);

			vk::Buffer vertexBuffers[] = { vertexBuffer.buffer };
			vk::DeviceSize vertexOffsets[] = { 0 };
			commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, vertexOffsets);

			commandBuffer.draw(vertexBuffer.count, 1, 0, 0);
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

	vk::CommandPoolCreateInfo poolInfo
	{
		.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = (uint32_t) graphicQueueFamily->index
	};

	commandPool = device.createCommandPool(poolInfo);
}

void CommandBuffer::CreateCommandBuffer()
{
	vk::CommandBufferAllocateInfo allocInfo
	{
		.commandPool = commandPool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1
	};

	commandBuffer = device.allocateCommandBuffers(allocInfo).front();
}
