#include <memory>
#include "VulkanContext.h"
#include "Utils/GLTFReader.h"
#include "Vulkan/DescriptorSets.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/SwapChain.h"
#include "RenderObjects/ColoredRenderObject.h"
#include "RenderObjects/TexturedRenderObject.h"
#include "RenderObjects/SimpleVertexedRenderObject.h"
#include "Objects/Primitives/PlaneObject.h"
#include "Objects/Primitives/BoundingBoxObject.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "Objects/Interfaces/Object.h"
#include "Utils/Deserializer.h"
#include "CAD/BoundingBoxTree.h"

class Scene
{
public:
	Scene()
	{ 
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(width, height, "Vulkan window", nullptr, nullptr);
		vulkanContext.Init(window);

		Deserializer deserializer(vulkanContext);
		//GLTFReader glTFReader("C:\\Users\\PC\\Desktop\\untitled\\scene.gltf");
		//GLTFReader glTFReader("C:\\Users\\PC\\Desktop\\untitled\\hard_monkey.gltf");
		GLTFReader glTFReader("C:\\Users\\PC\\Desktop\\untitled\\FirstContact.gltf");

		for (auto& serializedObject : glTFReader.serializedObjects)
		{
			auto object = deserializer.Deserialize(serializedObject);
			objects.push_back(std::move(object));
		}

		//auto plane = std::make_shared<PlaneObject>(vulkanContext,
		//	Vector3f(0., -1., 0.), Vector3f(0., 1., 0.));
		//objects.push_back(plane);

		boundingBoxTree = std::make_shared<BoundingBoxTree>(vulkanContext);
		boundingBoxTree->CreateBoundingBoxes(objects);
		objects.push_back(std::dynamic_pointer_cast<Object>(boundingBoxTree));

		auto& icosphere = objects[0];
		icosphere->position -= Vector3f(1, 0., 0.);

		// Center point
		auto object = deserializer.Deserialize(glTFReader.serializedObjects.front());
		object->position = Vector3f();
		object->scale = Vector3f(0.1, 0.1, 0.1);
		object->renderer->transformUniform.model = object->ComposeMatrix();
		object->renderer->UpdateTransformUniformBuffer();
		object->renderer->propertiesUniform.baseColor = Vector4f(1., 0., 1., 1.);
		object->renderer->UpdatePropertiesUniformBuffer();
		object->UpdateVertexBuffer();
		objects.push_back(std::move(object));
	}

	void Run()
	{
		while (!glfwWindowShouldClose(window)) {
			auto contactInfos = boundingBoxTree->ComposePairs();

			if (!contactInfos.front().contact) {
				auto& icosphere = objects[0];
				icosphere->position += Vector3f(0.001, 0., 0.);
				icosphere->renderer->transformUniform.model = icosphere->ComposeMatrix();
				icosphere->renderer->UpdateTransformUniformBuffer();
			}

			glfwPollEvents();
			vulkanContext.DrawFrame(objects);
		}
	}

	~Scene()
	{
		vulkanContext.Await();
		for (auto& object : objects) object->Dispose();
		vulkanContext.Dispose();
		glfwDestroyWindow(window);
		glfwTerminate();
	}

private:
	const uint32_t width = 1900;
	const uint32_t height = 1900;

	GLFWwindow* window;
	VulkanContext vulkanContext;

	std::vector<std::shared_ptr<Object>> objects;
	std::shared_ptr<BoundingBoxTree> boundingBoxTree;
};
