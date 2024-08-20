#pragma once
#include <memory>
#include "Scene.h"
#include "../Utils/GLTFReader.h"
#include "../Objects/Primitives/PlaneObject.h"
#include "../Renderers/PlaneRenderer.h"
#include "../Utils/Deserializer.h"
#include "../CAD/Desegmentator.h"
#include "../CAD/GeometryCreator.h"
#include "../Renderers/SimpleVertexedRenderer.h"

class VulkanContext;

class RigidScene : public Scene
{
public:
	RigidScene(VulkanContext& vulkanContext, std::vector<std::shared_ptr<Object>>& objects) : Scene(vulkanContext, objects)
	{
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

		Deserializer deserializer(vulkanContext);
		GLTFReader glTFReader("C:\\Users\\PC\\Desktop\\witch\\witch.gltf");
		//GLTFReader glTFReader("C:\\Users\\PC\\Desktop\\untitled\\Zombie_Schoolgirl_01.gltf");
		//GLTFReader glTFReader("C:\\Users\\PC\\Desktop\\untitled\\untitled.gltf");

		for (auto& serializedObject : glTFReader.serializedObjects)
		{
			auto object = deserializer.Deserialize(serializedObject);
			object->convexSegments = Desegmentator::ConvexSegments(*object->mesh);
			objects.push_back(std::move(object));
		}
	}
};
