#include <memory>
#include "VulkanContext.h"
#include "Utils/GLTFReader.h"
#include "Vulkan/DescriptorSets.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/SwapChain.h"
#include "RenderObjects/DeserializableObjects/ColoredRenderObject.h"
#include "RenderObjects/DeserializableObjects/TexturedRenderObject.h"
#include "RenderObjects/DeserializableObjects/PlaneVertexedRenderObject.h"
#include "RenderObjects/Primitives/EvenPlaneObject.h"
#include "RenderObjects/Primitives/BoundingBoxObject.h"

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

		//GLTFReader glTFReader("C:\\Users\\PC\\Desktop\\untitled\\scene.gltf");
		GLTFReader glTFReader("C:\\Users\\PC\\Desktop\\untitled\\hard_monkey.gltf");

		for (auto& deserializedObject : glTFReader.deserializedObjects)
		{
			std::unique_ptr<RenderObject> renderObject;
			if (deserializedObject.textureData.has_value())
				renderObject = std::make_unique<TexturedRenderObject>(vulkanContext, deserializedObject);
			else if (deserializedObject.hasColors)
				renderObject = std::make_unique<ColoredRenderObject>(vulkanContext, deserializedObject);
			else if (!deserializedObject.vertexData.empty())
				renderObject = std::make_unique<PlaneVertexedRenderObject>(vulkanContext, deserializedObject);
			renderObjects.push_back(std::move(renderObject));
		}

		auto plane = std::make_unique<EvenPlaneObject>(vulkanContext,
			//Vector3f(0., 0., 1.), Vector3f(0., 0., -1.));
			Vector3f(0., -1., 0.), Vector3f(0., 1., 0.));
		renderObjects.push_back(std::move(plane));

		auto boundingBox = std::make_unique<BoundingBoxObject>(vulkanContext,
			Vector3f(0., 0., 0.), Vector3f(1., 1., 1.));
		renderObjects.push_back(std::move(boundingBox));
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
	const uint32_t width = 2000;
	const uint32_t height = 2000;

	GLFWwindow* window;
	VulkanContext vulkanContext;

	std::vector<std::unique_ptr<RenderObject>> renderObjects;
};
