#include <vulkan/vulkan.hpp>

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