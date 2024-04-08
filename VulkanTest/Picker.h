#pragma once
#include "Objects/Interfaces/Object.h"
#include "Objects/Interfaces/MeshObject.h"
#include "Camera.h"

class Picker
{
public:
	void Init(VulkanContext& vulkanContext)
	{
		auto boundingBox = BoundingBox();
		boundingBox.aa = { -0.1, -0.1, -0.1 };
		boundingBox.bb = { 0.1, 0.1, 0.1 };

		auto boundingBoxObject = std::make_shared<BoundingBoxObject>(vulkanContext, boundingBox, false);
		boundingBox.renderBoundingBoxObject = boundingBoxObject;
		auto& boundingBoxObjectModel = boundingBoxObject->renderer->transformUniform.model;
		boundingBoxObjectModel = boundingBoxObjectModel;
		render = boundingBoxObject;
	}

	void Update(const std::vector<std::shared_ptr<Object>>& objects, const Camera& camera)
	{
		float minDist = std::numeric_limits<float>::max();

		for (auto& object : objects)
		{
			auto meshObject = std::dynamic_pointer_cast<MeshObject>(object);
			if (meshObject)
			{
				auto& mesh = *meshObject->mesh;
				auto& triangleBV = mesh.triangleBitVector;
				for (int i = 0; i < mesh.triangles.size(); ++i)
				{
					if (!triangleBV[i]) continue;
					auto points = mesh.TrianglePoints(i);

				}
			}
		}
	}

	void MouseMoved(Vector2f mousePos)
	{
		float offset = 5.;
		float dist = 5.;
		render->position.x = mousePos.x * dist;
		render->position.y = mousePos.y * dist;
		render->position.z = dist - offset;
	}

	std::shared_ptr<Object> pickedObj;
	std::shared_ptr<BoundingBoxObject> render;
	Vector3f pickedPos;
};
