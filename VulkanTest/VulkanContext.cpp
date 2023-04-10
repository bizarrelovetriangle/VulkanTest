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
#include "vulkan/VulkanAuxiliary.h"
#include "vulkan/DeviceController.h"
#include "vulkan/QueueFamilies.h"
#include "vulkan/Pipeline.h"
#include "vulkan/SwapChain.h"
#include "vulkan/RenderPass.h"
#include "vulkan/CommandBuffer.h"
#include "vulkan/VertexData.h"
#include "vulkan/VulkanBuffer.h"
#include "Utils/ObjReader.hpp"
#include "Utils/GLTFReader.h"
#include "Vulkan/ImageHelper.h"

void VulkanContext::Init(GLFWwindow* window)
{
    ValidationLayersInfo validationLayersInfo;
    vulkanAuxiliary = std::make_shared<VulkanAuxiliary>(validationLayersInfo);
    deviceController = std::make_shared<DeviceController>(vulkanAuxiliary->instance, validationLayersInfo);
    VkSurfaceKHR surfacePtr;
    auto res = glfwCreateWindowSurface(vulkanAuxiliary->instance, window, nullptr, &surfacePtr);
    surface = vk::SurfaceKHR(surfacePtr);
    queueFamilies = std::make_shared<QueueFamilies>(deviceController->physicalDevice, surface);

    auto graphicQueueFamily = std::find_if(std::begin(queueFamilies->queueFamilies), std::end(queueFamilies->queueFamilies),
        [](auto& family) { return family.flags.contains(vk::QueueFlagBits::eGraphics) && family.presentSupport; });
    deviceController->createDevice(*queueFamilies, { graphicQueueFamily->index });

    queueFamilies->graphicsQueue = deviceController->device.getQueue(graphicQueueFamily->index, 0);
    queueFamilies->presentQueue = deviceController->device.getQueue(graphicQueueFamily->index, 0);

    swapChain = std::make_shared<SwapChain>(*this);
    renderPass = std::make_shared<RenderPass>(deviceController->device, swapChain->swapChainImageFormat);
    swapChain->CreateFramebuffers(renderPass->renderPass);
    pipeline = std::make_shared<Pipeline>(deviceController->device, renderPass->renderPass, swapChain);
    commandBuffer = std::make_shared<CommandBuffer>(deviceController->device,
        queueFamilies, pipeline, swapChain, renderPass);
    imageHelper = std::make_shared<ImageHelper>(*this);

    ObjReader objReader("E:/Projects/VulkanTest/VulkanTest/Resources/Objects/SphereWithPlane/untitled.obj");


    {
        const std::vector<VertexData> vertexData = {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };

        vertexBuffer = std::make_shared<VulkanBuffer<VertexData>>(
            deviceController, vertexData, vk::BufferUsageFlagBits::eVertexBuffer);
    }

    {
        const std::vector<uint16_t> indices = {
            0, 1, 2
        };

        indexBuffer = std::make_shared<VulkanBuffer<uint16_t>>(
            deviceController, indices, vk::BufferUsageFlagBits::eIndexBuffer);
    }


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
    indexBuffer->Dispose();
    vertexBuffer->Dispose();

    vkDestroySemaphore(deviceController->device, imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(deviceController->device, renderFinishedSemaphore, nullptr);
    vkDestroyFence(deviceController->device, inFlightFence, nullptr);

    vkDestroySurfaceKHR(vulkanAuxiliary->instance, surface, nullptr);

    deviceController->Dispose();
    vulkanAuxiliary->Dispose();
}