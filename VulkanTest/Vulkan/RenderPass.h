#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.hpp>
#include <vector>
#include <string>
#include <fstream>

class RenderPass
{
public:
	RenderPass(const vk::Device& device, vk::Format swapChainImageFormat);
	void Dispose();

public:
	vk::RenderPass renderPass;

private:
	const vk::Device& device;
	vk::Format swapChainImageFormat;
};