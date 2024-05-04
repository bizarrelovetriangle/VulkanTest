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
#include "Vulkan/CommandBuffer.h"
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
        { queueFamilies->graphicQueueFamily, queueFamilies->presentQueueFamily, queueFamilies->transferQueueFamily };
    deviceController->createDevice(*queueFamilies, queueFamilyIndexes);

    queueFamilies->queueMap[queueFamilies->graphicQueueFamily] = deviceController->device.getQueue(queueFamilies->graphicQueueFamily, 0);
    queueFamilies->queueMap[queueFamilies->presentQueueFamily] = deviceController->device.getQueue(queueFamilies->presentQueueFamily, 0);
    queueFamilies->queueMap[queueFamilies->transferQueueFamily] = deviceController->device.getQueue(queueFamilies->transferQueueFamily, 0);

    commandBufferDispatcher = std::make_shared<CommandBufferDispatcher>(*this);

    swapChain = std::make_shared<SwapChain>(*this);
    renderPass = std::make_shared<RenderPass>(deviceController->device, swapChain->swapChainImageFormat);
    swapChain->CreateFramebuffers(renderPass->renderPass);
    commandBuffer = std::make_shared<CommandBuffer>(*this, deviceController->device,
        queueFamilies, swapChain, renderPass);

    vk::SemaphoreCreateInfo semaphoreInfo{};
    imageAvailableSemaphore = deviceController->device.createSemaphore(semaphoreInfo);
    renderFinishedSemaphore = deviceController->device.createSemaphore(semaphoreInfo);
    vk::FenceCreateInfo fenceInfo(vk::FenceCreateFlagBits::eSignaled);
    inFlightFence = deviceController->device.createFence(fenceInfo);

    std::span<CommonUniform> commonUniformSpan(&commonUniform, &commonUniform + 1);
    commonUniformBuffer = BufferData::Create<CommonUniform>(
        *this, commonUniformSpan, MemoryType::Universal, vk::BufferUsageFlagBits::eUniformBuffer);
}

VulkanContext::VulkanContext() = default;

VulkanContext::~VulkanContext() = default;

void VulkanContext::DrawFrame(std::vector<std::shared_ptr<Object>>& objects, const Camera& camera)
{
    deviceController->device.waitForFences(inFlightFence, VK_TRUE, UINT64_MAX);
    deviceController->device.resetFences(inFlightFence);

    uint32_t imageIndex = deviceController->device.acquireNextImageKHR(swapChain->swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE).value;

    RecordCommandBuffer(imageIndex, objects, camera);

    vk::Semaphore waitSemaphores[] = { imageAvailableSemaphore };
    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    vk::Semaphore signalSemaphores[] = { renderFinishedSemaphore };

    auto commandBuffers = { commandBuffer->commandBuffer };
    vk::SubmitInfo submitInfo(waitSemaphores, waitStages, commandBuffer->commandBuffer, signalSemaphores);
    queueFamilies->queueMap.at(queueFamilies->graphicQueueFamily).submit(submitInfo, inFlightFence);

    vk::PresentInfoKHR presentInfo(signalSemaphores, swapChain->swapChain, imageIndex);
    queueFamilies->queueMap.at(queueFamilies->presentQueueFamily).presentKHR(presentInfo);
}

void VulkanContext::RecordCommandBuffer(size_t imageIndex,
    const std::vector<std::shared_ptr<Object>>& objects, const Camera& camera)
{
    commonUniform.worldToView = camera.worldToView;
    commonUniform.viewToProj = camera.viewToProj;
    std::span<CommonUniform> commonUniformSpan(&commonUniform, &commonUniform + 1);
    commonUniformBuffer->FlushData(commonUniformSpan);

    commandBuffer->commandBuffer.reset();
    vk::CommandBufferBeginInfo beginInfo;
    commandBuffer->commandBuffer.begin(beginInfo);
        
    vk::Rect2D renderArea({ 0, 0 }, swapChain->swapChainExtent);

    vk::ClearColorValue clearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
    vk::ClearDepthStencilValue clearDepthStencilValue(1., 0.);
    std::vector<vk::ClearValue> clearColor{ clearColorValue, clearDepthStencilValue };

    vk::RenderPassBeginInfo renderPassInfo(
        renderPass->renderPass, swapChain->swapChainFramebuffers[imageIndex],
        renderArea, clearColor);

    commandBuffer->commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    auto viewport = swapChain->CreateViewport();
    auto scissors = swapChain->CreateScissors();
    commandBuffer->commandBuffer.setViewport(0, 1, &viewport);
    commandBuffer->commandBuffer.setScissor(0, 1, &scissors);

    RenderVisitor renderVisitor(*this, *commandBuffer, imageIndex);

    for (auto& object : objects)
    {
        object->Render(renderVisitor);
    }

    commandBuffer->commandBuffer.endRenderPass();
    commandBuffer->commandBuffer.end();
}

void VulkanContext::Await()
{
    vkDeviceWaitIdle(deviceController->device);
}

void VulkanContext::Dispose()
{
    commonUniformBuffer->Dispose();

    swapChain->Dispose();
    commandBufferDispatcher->Dispose();
    commandBuffer->Dispose();
    renderPass->Dispose();

    vkDestroySemaphore(deviceController->device, imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(deviceController->device, renderFinishedSemaphore, nullptr);
    vkDestroyFence(deviceController->device, inFlightFence, nullptr);

    vkDestroySurfaceKHR(vulkanAuxiliary->instance, surface, nullptr);

    deviceController->Dispose();
    vulkanAuxiliary->Dispose();
}