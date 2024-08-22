#pragma once
#include <vulkan/vulkan.hpp>
#include "Math/Matrix4.h"

class CommandBuffer;
class DeviceController;
class VulkanAuxiliary;
class QueueFamilies;
class SwapChain;
class RenderPass;
class Pipeline;
class VertexData;
class Object;
struct GLFWwindow;
template <class T>
class BufferMemory;
class CommandBufferDispatcher;
class Camera;
class BufferData;

class CommonUniform
{
public:
    alignas(16) Matrix4 worldToView;
    alignas(16) Matrix4 viewToProj;
};

class VulkanContext
{
public:
    void Init(GLFWwindow* window);
   VulkanContext();
    ~VulkanContext();
    void DrawFrame(std::vector<std::shared_ptr<Object>>& objects, const Camera& camera);
    void RecordCommandBuffer(size_t imageIndex, const std::vector<std::shared_ptr<Object>>& objects, const Camera& camera);
    void Await();
    void Dispose();

    std::shared_ptr<CommandBuffer> commandBuffer;
    std::shared_ptr<DeviceController> deviceController;

    GLFWwindow* window = nullptr;
    vk::SurfaceKHR surface;
    std::shared_ptr<VulkanAuxiliary> vulkanAuxiliary;
    std::shared_ptr<QueueFamilies> queueFamilies;
    std::shared_ptr<SwapChain> swapChain;
    std::shared_ptr<RenderPass> renderPass;
    std::shared_ptr<CommandBufferDispatcher> commandBufferDispatcher;

    CommonUniform commonUniform;
    std::unique_ptr<BufferData> commonUniformBuffer;

private:
    vk::Semaphore computeCompleteSemaphore;
    vk::Semaphore imageAvailableSemaphore;
    vk::Semaphore renderFinishedSemaphore;
    vk::Fence inFlightFence;
};