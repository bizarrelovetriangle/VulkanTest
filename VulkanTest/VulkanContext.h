#pragma once;

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.hpp>

class CommandBuffer;
class DeviceController;
class VulkanAuxiliary;
class QueueFamilies;
class SwapChain;
class RenderPass;
class Pipeline;
class VertexData;
class RenderObject;
class GLFWwindow;
template <class T>
class BufferMemory;

class VulkanContext
{
public:
    void Init(GLFWwindow* window);
    void DrawFrame(std::vector<std::unique_ptr<RenderObject>>& renderObjects);
    void Await();
    void Dispose();

    std::shared_ptr<CommandBuffer> commandBuffer;
    std::shared_ptr<DeviceController> deviceController;

    GLFWwindow* window;
    vk::SurfaceKHR surface;
    std::shared_ptr<VulkanAuxiliary> vulkanAuxiliary;
    std::shared_ptr<QueueFamilies> queueFamilies;
    std::shared_ptr<SwapChain> swapChain;
    std::shared_ptr<RenderPass> renderPass;
    std::shared_ptr<Pipeline> pipeline;

private:
    vk::Semaphore imageAvailableSemaphore;
    vk::Semaphore renderFinishedSemaphore;
    vk::Fence inFlightFence;
};