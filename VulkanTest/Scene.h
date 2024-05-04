#include <memory>
#include "VulkanContext.h"
#include "Utils/GLTFReader.h"
#include "Vulkan/DescriptorSets.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/SwapChain.h"
#include "Renderers/ColoredRenderer.h"
#include "Renderers/TexturedRenderer.h"
#include "Renderers/SimpleVertexedRenderer.h"
#include "Renderers/PlaneRenderer.h"
#include "Objects/Primitives/PlaneObject.h"
#include "Objects/Primitives/BoundingBoxObject.h"
#include "Camera.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "Objects/Interfaces/Object.h"
#include "Utils/Deserializer.h"
#include "CAD/BoundingBoxTree.h"
#include "Picker.h"

class Scene
{
public:
	Scene()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(windowSize.x, windowSize.y, "Vulkan window", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetKeyCallback(window, KeyCallback);
		glfwSetScrollCallback(window, ScrollCallback);
		glfwSetCursorPosCallback(window, CursorPositionCallback);
		glfwSetMouseButtonCallback(window, MouseClickCallback);

		vulkanContext.Init(window);

		Deserializer deserializer(vulkanContext);
		//GLTFReader glTFReader("C:\\Users\\PC\\Desktop\\untitled\\scene.gltf");
		GLTFReader glTFReader("C:\\Users\\PC\\Desktop\\untitled\\hard_monkey.gltf");
		//GLTFReader glTFReader("C:\\Users\\PC\\Desktop\\untitled\\FirstContact.gltf");

		for (auto& serializedObject : glTFReader.serializedObjects)
		{
			auto object = deserializer.Deserialize(serializedObject);
			objects.push_back(std::move(object));
		}

		auto plane = std::make_shared<PlaneObject>(vulkanContext,
			Vector3f(0., -1., 0.), Vector3f(0., 1., 0.));
		plane->scale = plane->scale * 300.;
		plane->interactive = false;
		auto planeRenderer = (PlaneRenderer*)plane->renderer.get();
		planeRenderer->evenPlaneObjectUniform.gridScale = plane->scale;
		planeRenderer->evenPlaneObjectUniform.gridded = true;
		planeRenderer->UpdatePlaneUniformBuffer();
		objects.push_back(plane);

		auto center = std::make_unique<MeshObject>(
			GeometryCreator::CreateIcosphere(0.02, 1), std::make_unique<SimpleVertexedRenderer>(vulkanContext));
		center->interactive = false;
		objects.push_back(std::move(center));

		boundingBoxTree = std::make_shared<BoundingBoxTree>(vulkanContext);
		boundingBoxTree->CreateBoundingBoxes(objects);
		objects.push_back(boundingBoxTree);

		picker.Init(vulkanContext, camera);
		if (picker.pointer)
			objects.push_back(picker.pointer);
	}

	void Run()
	{
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();

			boundingBoxTree->UpdateBoundingBoxes(objects);
			//auto contactInfos = boundingBoxTree->ComposePairs();

			picker.Update(objects);
			//camera.rotatePoint = picker.pickedPos;
			auto copy = std::vector(objects.begin() + 0, objects.end());
			vulkanContext.DrawFrame(copy, camera);
		}
	}

	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		auto scene = (Scene*)glfwGetWindowUserPointer(window);
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		if (key == GLFW_KEY_T && action == GLFW_PRESS)
			scene->scrollMode = !scene->scrollMode;
	}

	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		auto scene = (Scene*)glfwGetWindowUserPointer(window);
		Vector2f offset(xoffset, yoffset);

		if (scene->scrollMode)
			scene->camera.Scrolled(offset);
		else
			scene->camera.Zoom(-yoffset);

		scene->picker.UpdatePicked();
	}

	static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
	{
		auto scene = (Scene*)glfwGetWindowUserPointer(window);
		Vector2f mousePos = Vector2i(xpos, ypos) - scene->windowSize / 2;
		mousePos.y = -mousePos.y;

		mousePos = { mousePos.x / (scene->windowSize.x / 2), mousePos.y / (scene->windowSize.y / 2) };

		scene->camera.MouseMoved(mousePos);
		scene->picker.MouseMoved(mousePos);
		scene->picker.UpdatePicked();
	}

	static void MouseClickCallback(GLFWwindow* window, int button, int action, int mods)
	{
		auto scene = (Scene*)glfwGetWindowUserPointer(window);
		bool down = action == GLFW_PRESS;

		if (button == GLFW_MOUSE_BUTTON_RIGHT)
			scene->camera.MouseRightDown(down);
		if (button == GLFW_MOUSE_BUTTON_MIDDLE)
			scene->camera.MouseMiddleDown(down);
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			if (scene->scrollMode)
				scene->camera.MouseMiddleDown(down);
			else
				if (down) scene->picker.Pick();
				else scene->picker.UnPick();
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
	const Vector2i windowSize = Vector2i(1900, 1900);

	GLFWwindow* window;
	VulkanContext vulkanContext;

	bool scrollMode = false;
	Camera camera;
	Picker picker;

	std::vector<std::shared_ptr<Object>> objects;
	std::shared_ptr<BoundingBoxTree> boundingBoxTree;
};
