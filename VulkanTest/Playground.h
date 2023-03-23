#pragma once;

#define VK_USE_PLATFORM_WIN32_KHR
#define NTDDI_VERSION NTDDI_WIN10_RS1 // work around linker failure MapViewOfFileNuma2@36

#define VK_HEADER_VERSION 239

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <iostream>
#include <vector>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <limits>
#include <optional>
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
#include "Utils/GLTFReader.hpp"

class Playground
{
public:
    Playground()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(width, height, "Vulkan window", nullptr, nullptr);
        
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
        
        swapChain = std::make_shared<SwapChain>(deviceController, surface, window);
        renderPass = std::make_shared<RenderPass>(deviceController->device, swapChain->swapChainImageFormat);
        swapChain->CreateFramebuffers(renderPass->renderPass);
        pipeline = std::make_shared<Pipeline>(deviceController->device, renderPass->renderPass, swapChain);
        commandBuffer = std::make_shared<CommandBuffer>(deviceController->device,
            queueFamilies, pipeline, swapChain, renderPass);


        ObjReader objReader("E:/Projects/VulkanTest/VulkanTest/Resources/Objects/SphereWithPlane/untitled.obj");
        GLTFReader glTFReader("C:\\Users\\Dell\\Desktop\\untitled\\hard.gltf");

        {
            auto& vertices = glTFReader.renderObjects.begin()->second.vertexData;
            objVertexBuffer = std::make_shared<VulkanBuffer<RenderObjectVertexData>>(
                deviceController, vertices, vk::BufferUsageFlagBits::eVertexBuffer);
        }

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
        vk::FenceCreateInfo fenceInfo{ .flags = vk::FenceCreateFlagBits::eSignaled };
        inFlightFence = deviceController->device.createFence(fenceInfo);
    }

    void Run() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            DrawFrame();
        }
        
        vkDeviceWaitIdle(deviceController->device);
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void DrawFrame()
    {
        deviceController->device.waitForFences({ inFlightFence }, VK_TRUE, UINT64_MAX);
        deviceController->device.resetFences({ inFlightFence });

        uint32_t imageIndex = deviceController->device.acquireNextImageKHR(swapChain->swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE).value;

        commandBuffer->Reset();
        commandBuffer->RecordCommandBuffer(imageIndex, *objVertexBuffer);

        vk::Semaphore waitSemaphores[] = { imageAvailableSemaphore };
        vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
        vk::Semaphore signalSemaphores[] = { renderFinishedSemaphore };

        vk::SubmitInfo submitInfo
        {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores,
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer->commandBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signalSemaphores
        };

        queueFamilies->graphicsQueue.submit(submitInfo, inFlightFence);

        vk::SwapchainKHR swapChains[] = { swapChain->swapChain };

        vk::PresentInfoKHR presentInfo
        {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signalSemaphores,
            .swapchainCount = 1,
            .pSwapchains = swapChains,
            .pImageIndices = &imageIndex
        };

        queueFamilies->presentQueue.presentKHR(presentInfo);
    }

    ~Playground()
    {
        swapChain->Dispose();
        commandBuffer->Dispose();
        pipeline->Dispose();
        renderPass->Dispose();
        indexBuffer->Dispose();
        vertexBuffer->Dispose();
        objVertexBuffer->Dispose();

        vkDestroySemaphore(deviceController->device, imageAvailableSemaphore, nullptr);
        vkDestroySemaphore(deviceController->device, renderFinishedSemaphore, nullptr);
        vkDestroyFence(deviceController->device, inFlightFence, nullptr);

        vkDestroySurfaceKHR(vulkanAuxiliary->instance, surface, nullptr);

        deviceController->Dispose();
        vulkanAuxiliary->Dispose();
    }

private:
    const uint32_t width = 1200;
    const uint32_t height = 1200;

    vk::SurfaceKHR surface;
    GLFWwindow* window;
    std::shared_ptr<VulkanAuxiliary> vulkanAuxiliary;
    std::shared_ptr<DeviceController> deviceController;
    std::shared_ptr<QueueFamilies> queueFamilies;
    std::shared_ptr<SwapChain> swapChain;
    std::shared_ptr<RenderPass> renderPass;
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<CommandBuffer> commandBuffer;

    std::shared_ptr<VulkanBuffer<uint16_t>> indexBuffer;
    std::shared_ptr<VulkanBuffer<VertexData>> vertexBuffer;

    std::shared_ptr<VulkanBuffer<RenderObjectVertexData>> objVertexBuffer;

    vk::Semaphore imageAvailableSemaphore;
    vk::Semaphore renderFinishedSemaphore;
    vk::Fence inFlightFence;
};