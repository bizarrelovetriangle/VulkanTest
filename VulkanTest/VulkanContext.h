#pragma once
#include <vulkan/vulkan.hpp>

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

class VulkanContext
{
public:
    void Init(GLFWwindow* window);
    ~VulkanContext();
    void DrawFrame(std::vector<std::shared_ptr<Object>>& objects, const Camera& camera);
    void RecordCommandBuffer(size_t imageIndex, const std::vector<std::shared_ptr<Object>>& objects, const Camera& camera);
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
    std::shared_ptr<CommandBufferDispatcher> commandBufferDispatcher;

private:
    vk::Semaphore imageAvailableSemaphore;
    vk::Semaphore renderFinishedSemaphore;
    vk::Fence inFlightFence;
};