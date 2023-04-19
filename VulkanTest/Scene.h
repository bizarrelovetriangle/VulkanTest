
#include <memory>
#include "VulkanContext.h"
#include "Utils/GLTFReader.h"
#include "Primitives/RenderObject.h"
#include "Vulkan/DescriptorSets.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/SwapChain.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#undef LoadImage;

class Scene
{
public:
	Scene()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(width, height, "Vulkan window", nullptr, nullptr);
		vulkanContext.Init(window);

		GLTFReader glTFReader("C:\\Users\\Dell\\Downloads\\girl_speedsculpt\\scene.gltf");
		//GLTFReader glTFReader("C:\\Users\\Dell\\Desktop\\untitled\\hard_monkey.gltf");

		renderObjects = std::move(glTFReader.renderObjects);

		for (auto& renderObject : renderObjects)
		{
			renderObject->vertexBuffer = std::make_unique<BufferMemory<RenderObjectVertexData>>(
				vulkanContext, renderObject->vertexData, MemoryType::DeviceLocal, vk::BufferUsageFlagBits::eVertexBuffer);

			renderObject->descriptorSets = std::make_unique<DescriptorSets>(
				vulkanContext, vulkanContext.pipeline->descriptorSetLayout, vulkanContext.swapChain->frameCount);

			if (renderObject->textureData)
			{
				auto& [resolution, imageData] = *renderObject->textureData;
				renderObject->textureBuffer = std::make_unique<ImageMemory>(
					vulkanContext, resolution, vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eSampled, vk::ImageAspectFlagBits::eColor,
					MemoryType::Universal);
				renderObject->textureBuffer->FlushData(imageData);
				renderObject->textureBuffer->TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
			}
			else
			{
				auto [resolution, imageData] = ImageMemory::LoadImage("E:/Images/testImage.jpeg");
				renderObject->textureBuffer = std::make_unique<ImageMemory>(
					vulkanContext, resolution, vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eSampled, vk::ImageAspectFlagBits::eColor,
					MemoryType::Universal);
				renderObject->textureBuffer->TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
			}

			std::span<RenderObjectUniform> uniformSpan(&renderObject->uniform, &renderObject->uniform + 1);
			renderObject->uniformBuffer = std::make_unique<BufferMemory<RenderObjectUniform>>(
				vulkanContext, uniformSpan, MemoryType::Universal, vk::BufferUsageFlagBits::eUniformBuffer);

			renderObject->descriptorSets->UpdateDescriptor(*renderObject->uniformBuffer, *renderObject->textureBuffer);
		}
	}

	void Run()
	{
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			vulkanContext.DrawFrame(renderObjects);
		}
	}

	~Scene()
	{
		vulkanContext.Await();
		for (auto& renderObject : renderObjects) renderObject->Dispose();
		vulkanContext.Dispose();
		glfwDestroyWindow(window);
		glfwTerminate();
	}

private:
	const uint32_t width = 1200;
	const uint32_t height = 1200;

	GLFWwindow* window;
	VulkanContext vulkanContext;

	std::vector<std::unique_ptr<RenderObject>> renderObjects;
};
