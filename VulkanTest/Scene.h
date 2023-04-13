
#include <memory>
#include "VulkanContext.h"
#include "Utils/GLTFReader.h"
#include "Primitives/RenderObject.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

class Scene
{
public:
	Scene()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(width, height, "Vulkan window", nullptr, nullptr);
		vulkanContext.Init(window);

		//GLTFReader glTFReader("C:\\Users\\Dell\\Downloads\\girl_speedsculpt\\scene.gltf");
		GLTFReader glTFReader("C:\\Users\\Dell\\Desktop\\untitled\\hard_monkey.gltf");

		renderObjects = std::move(glTFReader.renderObjects);

		for (auto& renderObject : renderObjects)
		{
			renderObject->vertexBuffer = std::make_unique<BufferMemory<RenderObjectVertexData>>(
				vulkanContext, renderObject->vertexData, MemoryType::DeviceLocal, vk::BufferUsageFlagBits::eVertexBuffer);
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
		for (auto& renderObject : renderObjects) renderObject->vertexBuffer->Dispose();
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
