#pragma once
#include <memory>
#include "VulkanContext.h"
#include "Utils/GLTFReader.h"
#include "Vulkan/DescriptorSets.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/SwapChain.h"
#include "Camera.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "Objects/Interfaces/Object.h"
#include "Utils/Deserializer.h"
#include "CAD/BoundingBoxTree.h"
#include "Picker.h"
#include "CAD/Desegmentator.h"
#include "Scenes/FluidScene.h"
#include "Scenes/RigidScene.h"

class Application
{
public:
	Application()
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

		scene = std::make_unique<RigidScene>(vulkanContext, objects);

		boundingBoxTree = std::make_shared<BoundingBoxTree>(vulkanContext);
		objects.push_back(boundingBoxTree);

		picker.Init(vulkanContext, camera);
		if (picker.pointer)
			objects.push_back(picker.pointer);
	}

	void Run()
	{
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();

			boundingBoxTree->UpdateTree(objects);
			auto contactInfos = boundingBoxTree->ComposePairs();

			picker.Update(*boundingBoxTree);
			//camera.rotatePoint = picker.pickedPos;
			vulkanContext.DrawFrame(objects, camera);
		}
	}

	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		auto app = (Application*)glfwGetWindowUserPointer(window);
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		if (key == GLFW_KEY_T && action == GLFW_PRESS)
			app->scrollMode = !app->scrollMode;
	}

	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		auto app = (Application*)glfwGetWindowUserPointer(window);
		Vector2f offset(xoffset, yoffset);

		if (app->scrollMode)
			app->camera.Scrolled(offset);
		else
			app->camera.Zoom(-yoffset);

		app->picker.UpdatePicked();
	}

	static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
	{
		auto app = (Application*)glfwGetWindowUserPointer(window);
		Vector2f mousePos = Vector2i(xpos, ypos) - app->windowSize / 2;
		mousePos.y = -mousePos.y;

		mousePos = { mousePos.x / (app->windowSize.x / 2), mousePos.y / (app->windowSize.y / 2) };

		app->camera.MouseMoved(mousePos);
		app->picker.MouseMoved(mousePos);
		app->picker.UpdatePicked();
	}

	static void MouseClickCallback(GLFWwindow* window, int button, int action, int mods)
	{
		auto app = (Application*)glfwGetWindowUserPointer(window);
		bool down = action == GLFW_PRESS;

		if (button == GLFW_MOUSE_BUTTON_RIGHT)
			app->camera.MouseRightDown(down);
		if (button == GLFW_MOUSE_BUTTON_MIDDLE)
			app->camera.MouseMiddleDown(down);
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			if (app->scrollMode)
				app->camera.MouseMiddleDown(down);
			else
				if (down) app->picker.Pick();
				else app->picker.UnPick();
		}
	}

	~Application()
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
	std::unique_ptr<Scene> scene;

	bool scrollMode = false;
	Camera camera;
	Picker picker;

	std::vector<std::shared_ptr<Object>> objects;
	std::shared_ptr<BoundingBoxTree> boundingBoxTree;
};
