#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define NTDDI_VERSION NTDDI_WIN10_RS1 // work around linker failure MapViewOfFileNuma2@36

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <iostream>
#include <vector>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <limits>
#include <optional>
#include "VulkanContext.h"
#include "Vulkan/VulkanAuxiliary.h"
#include "Vulkan/DeviceController.h"
#include "Vulkan/QueueFamilies.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/SwapChain.h"
#include "Vulkan/RenderPass.h"
#include "Utils/GLTFReader.h"

#include "Vulkan/CommandBufferDispatcher.h"
#include "Objects/Interfaces/Object.h"
#include "Renderers/Interfaces/Renderer.h"
#include "Camera.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "RenderVisitor.h"
#include "Vulkan/Data/BufferData.h"
#include "Objects/FluidObject.h"

void VulkanContext::Init(GLFWwindow* window)
{
    ValidationLayersInfo validationLayersInfo;
    vulkanAuxiliary = std::make_shared<VulkanAuxiliary>(validationLayersInfo);
    deviceController = std::make_shared<DeviceController>(vulkanAuxiliary->instance, validationLayersInfo);
    VkSurfaceKHR surfacePtr;
    auto res = glfwCreateWindowSurface(vulkanAuxiliary->instance, window, nullptr, &surfacePtr);
    surface = vk::SurfaceKHR(surfacePtr);
    queueFamilies = std::make_shared<QueueFamilies>(deviceController->physicalDevice, surface);

    std::vector<uint32_t> queueFamilyIndexes =
        { queueFamilies->graphicQueueFamily, queueFamilies->presentQueueFamily, queueFamilies->transferQueueFamily, queueFamilies->computeQueueFamily };
    deviceController->createDevice(*queueFamilies, queueFamilyIndexes);

    queueFamilies->queueMap[queueFamilies->graphicQueueFamily] = deviceController->device.getQueue(queueFamilies->graphicQueueFamily, 0);
    queueFamilies->queueMap[queueFamilies->presentQueueFamily] = deviceController->device.getQueue(queueFamilies->presentQueueFamily, 0);
    queueFamilies->queueMap[queueFamilies->transferQueueFamily] = deviceController->device.getQueue(queueFamilies->transferQueueFamily, 0);
    queueFamilies->queueMap[queueFamilies->computeQueueFamily] = deviceController->device.getQueue(queueFamilies->computeQueueFamily, 0);

    commandBufferDispatcher = std::make_shared<CommandBufferDispatcher>(*this);

    swapChain = std::make_shared<SwapChain>(*this);
    renderPass = std::make_shared<RenderPass>(deviceController->device, swapChain->swapChainImageFormat);
    swapChain->CreateFramebuffers(renderPass->renderPass);

    vk::SemaphoreCreateInfo semaphoreInfo{};
    computeCompleteSemaphore = deviceController->device.createSemaphore(semaphoreInfo);
    imageAvailableSemaphore = deviceController->device.createSemaphore(semaphoreInfo);
    renderFinishedSemaphore = deviceController->device.createSemaphore(semaphoreInfo);
    vk::FenceCreateInfo fenceInfo(vk::FenceCreateFlagBits::eSignaled);
    inFlightFence = deviceController->device.createFence(fenceInfo);

    commonUniformBuffer = BufferData::Create(*this, commonUniform, MemoryType::Universal, vk::BufferUsageFlagBits::eUniformBuffer);
    renderCommandBuffer = commandBufferDispatcher->GetCommandBuffer(queueFamilies->graphicQueueFamily);
}

VulkanContext::VulkanContext() = default;

VulkanContext::~VulkanContext() = default;

void VulkanContext::DrawFrame(std::vector<std::shared_ptr<Object>>& objects, const Camera& camera)
{
    auto _ = deviceController->device.waitForFences(inFlightFence, VK_TRUE, UINT64_MAX);
    deviceController->device.resetFences(inFlightFence);

    commonUniform.worldToView = camera.worldToView;
    commonUniform.viewToProj = camera.viewToProj;
    commonUniformBuffer->FlushData(commonUniform);

    uint32_t imageIndex = deviceController->device.acquireNextImageKHR(swapChain->swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE).value;

    {
        auto commandBuffer = commandBufferDispatcher->GetCommandBuffer(queueFamilies->computeQueueFamily);
        auto fence = commandBufferDispatcher->GetFence();

        vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        commandBuffer.begin(beginInfo);

        for (auto& object : objects)
        {
            if (auto fluidObject = std::dynamic_pointer_cast<FluidObject>(object))
            {
                fluidObject->Run(commandBuffer, imageIndex);
            }
        }

        commandBuffer.end();

        vk::SubmitInfo submitInfo({}, {}, commandBuffer, computeCompleteSemaphore);
        auto& queue = queueFamilies->queueMap.at(queueFamilies->computeQueueFamily);
        queue.submit(submitInfo, fence);
        commandBufferDispatcher->SubmitFence(fence, queueFamilies->computeQueueFamily, commandBuffer);
    }

    auto waitSemaphores = std::vector{ imageAvailableSemaphore, computeCompleteSemaphore };
    auto waitStages = std::vector<vk::PipelineStageFlags>
        { vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eComputeShader };

    {
        vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        renderCommandBuffer.begin(beginInfo);
        RecordCommandBuffer(imageIndex, renderCommandBuffer, objects, camera);
        renderCommandBuffer.end();

        vk::SubmitInfo submitInfo(waitSemaphores, waitStages, renderCommandBuffer, renderFinishedSemaphore);
        auto& queue = queueFamilies->queueMap.at(queueFamilies->graphicQueueFamily);
        queue.submit(submitInfo, inFlightFence);
    }

    vk::PresentInfoKHR presentInfo(renderFinishedSemaphore, swapChain->swapChain, imageIndex);
    _ = queueFamilies->queueMap.at(queueFamilies->presentQueueFamily).presentKHR(presentInfo);
}

void VulkanContext::RecordCommandBuffer(size_t imageIndex, vk::CommandBuffer& commandBuffer,
    const std::vector<std::shared_ptr<Object>>& objects, const Camera& camera)
{
    vk::Rect2D renderArea({ 0, 0 }, swapChain->swapChainExtent);

    vk::ClearColorValue clearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
    vk::ClearDepthStencilValue clearDepthStencilValue(1., 0.);
    std::vector<vk::ClearValue> clearColor{ clearColorValue, clearDepthStencilValue };

    vk::RenderPassBeginInfo renderPassInfo(
        renderPass->renderPass, swapChain->swapChainFramebuffers[imageIndex],
        renderArea, clearColor);

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    auto viewport = swapChain->CreateViewport();
    auto scissors = swapChain->CreateScissors();
    commandBuffer.setViewport(0, 1, &viewport);
    commandBuffer.setScissor(0, 1, &scissors);

    RenderVisitor renderVisitor(*this, commandBuffer, imageIndex);

    for (auto& object : objects)
    {
        if (object->visible) {
            object->Render(renderVisitor);
        }
    }

    commandBuffer.endRenderPass();
}

void VulkanContext::Await()
{
    deviceController->device.waitIdle();
}

void VulkanContext::Dispose()
{
    commonUniformBuffer->Dispose();

    swapChain->Dispose();
    commandBufferDispatcher->Dispose();
    renderPass->Dispose();

    deviceController->device.destroySemaphore(imageAvailableSemaphore);
    deviceController->device.destroySemaphore(renderFinishedSemaphore);
    deviceController->device.destroySemaphore(computeCompleteSemaphore);
    deviceController->device.destroyFence(inFlightFence);
    vulkanAuxiliary->instance.destroySurfaceKHR(surface);

    deviceController->Dispose();
    vulkanAuxiliary->Dispose();
}