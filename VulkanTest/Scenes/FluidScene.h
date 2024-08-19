#pragma once
#include "Scene.h"

class FluidScene : public Scene
{
public:
	FluidScene(VulkanContext& vulkanContext, std::vector<std::shared_ptr<Object>>& objects) : Scene(vulkanContext, objects)
	{
	
	}
};