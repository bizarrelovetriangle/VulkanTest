#pragma once;

#define VK_USE_PLATFORM_WIN32_KHR
#define NTDDI_VERSION NTDDI_WIN10_RS1 // work around linker failure MapViewOfFileNuma2@36

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

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
#include "Vulkan/VertexData.h"
#include "Vulkan/Memory/BufferMemory.h"
#include "Utils/ObjReader.hpp"
#include "Utils/GLTFReader.h"
#include "Vulkan/Memory/ImageMemory.h"

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
        { queueFamilies->graphicQueueFamilyIndex, queueFamilies->presentQueueFamilyIndex, queueFamilies->transferQueueFamilyIndex };
    deviceController->createDevice(*queueFamilies, queueFamilyIndexes);

    queueFamilies->graphicsQueue = deviceController->device.getQueue(queueFamilies->graphicQueueFamilyIndex, 0);
    queueFamilies->presentQueue = deviceController->device.getQueue(queueFamilies->presentQueueFamilyIndex, 0);
    queueFamilies->transferQueue = deviceController->device.getQueue(queueFamilies->transferQueueFamilyIndex, 0);

    swapChain = std::make_shared<SwapChain>(*this);
    renderPass = std::make_shared<RenderPass>(deviceController->device, swapChain->swapChainImageFormat);
    swapChain->CreateFramebuffers(renderPass->renderPass);
    pipeline = std::make_shared<Pipeline>(deviceController->device, renderPass->renderPass, swapChain);
    commandBuffer = std::make_shared<CommandBuffer>(deviceController->device,
        queueFamilies, pipeline, swapChain, renderPass);

    vk::SemaphoreCreateInfo semaphoreInfo{};
    imageAvailableSemaphore = deviceController->device.createSemaphore(semaphoreInfo);
    renderFinishedSemaphore = deviceController->device.createSemaphore(semaphoreInfo);
    vk::FenceCreateInfo fenceInfo(vk::FenceCreateFlagBits::eSignaled);
    inFlightFence = deviceController->device.createFence(fenceInfo);
}

void VulkanContext::DrawFrame(std::vector<std::unique_ptr<RenderObject>>& renderObjects)
{
    deviceController->device.waitForFences({ inFlightFence }, VK_TRUE, UINT64_MAX);
    deviceController->device.resetFences({ inFlightFence });

    uint32_t imageIndex = deviceController->device.acquireNextImageKHR(swapChain->swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE).value;

    commandBuffer->Reset();
    commandBuffer->RecordCommandBuffer(imageIndex, renderObjects);

    vk::Semaphore waitSemaphores[] = { imageAvailableSemaphore };
    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    vk::Semaphore signalSemaphores[] = { renderFinishedSemaphore };

    auto commandBuffers = { commandBuffer->commandBuffer };
    vk::SubmitInfo submitInfo(imageAvailableSemaphore, waitStages, commandBuffer->commandBuffer, signalSemaphores);
    queueFamilies->graphicsQueue.submit(submitInfo, inFlightFence);

    vk::PresentInfoKHR presentInfo(signalSemaphores, swapChain->swapChain, imageIndex);
    queueFamilies->presentQueue.presentKHR(presentInfo);
}

void VulkanContext::Await()
{

    vkDeviceWaitIdle(deviceController->device);
}

void VulkanContext::Dispose()
{
    swapChain->Dispose();
    commandBuffer->Dispose();
    pipeline->Dispose();
    renderPass->Dispose();

    vkDestroySemaphore(deviceController->device, imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(deviceController->device, renderFinishedSemaphore, nullptr);
    vkDestroyFence(deviceController->device, inFlightFence, nullptr);

    vkDestroySurfaceKHR(vulkanAuxiliary->instance, surface, nullptr);

    deviceController->Dispose();
    vulkanAuxiliary->Dispose();
}