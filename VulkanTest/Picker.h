#pragma once
#include "Objects/Interfaces/Object.h"
#include "Objects/Interfaces/MeshObject.h"
#include "Camera.h"
#include "CAD/GeometryCreator.h"

class Picker
{
public:
	void Init(VulkanContext& vulkanContext)
	{
		auto pointerRenderer = std::make_unique<SimpleVertexedRenderer>(vulkanContext);
		auto mesh = GeometryCreator::CreateIcosphere(0.1, 2);
		pointer = std::make_shared<MeshObject>(std::move(mesh), std::move(pointerRenderer));
		pointer->UpdateVertexBuffer();
		pointer->name = "pointer";
	}

	// Invert view!
	void Update(const std::vector<std::shared_ptr<Object>>& objects, const Camera& camera)
	{
		auto inverseView = camera.view.Inverse();
		auto segmentA = Vector3f(inverseView * Vector4f(Vector3f::Zero(), 1.));
		auto segmentB = Vector3f(inverseView * Vector4f(mouseDirection * 1000, 1.));

		pointer->position = segmentA;
		focusedPos = std::nullopt;
		focusedObj = nullptr;

		float minDist = (std::numeric_limits<float>::max)();
		Vector3f pos;
		std::shared_ptr<Object> obj;

		for (auto& object : objects)
		{
			if (object->name == "pointer") continue;

			auto meshObject = std::dynamic_pointer_cast<MeshObject>(object);
			if (meshObject)
			{
				auto objMatrix = object->ComposeMatrix();
				auto& mesh = *meshObject->mesh;
				auto& triangleBV = mesh.triangleBitVector;
				for (int i = 0; i < mesh.triangles.size(); ++i)
				{
					if (!triangleBV[i]) continue;
					auto points = mesh.TrianglePoints(i);
					for (auto& point : points)
						point = Vector3f(objMatrix * Vector4f(point, 1.));

					float ratio = 0.;
					Vector3f intersectPoint;
					if (GeometryFunctions::SegmentTriangleIntersetion(
						segmentA, segmentB, points[0], points[1], points[2], intersectPoint, &ratio))
					{
						if (ratio < minDist)
						{
							minDist = ratio;
							pos = intersectPoint;
							obj = object;
						}
					}
				}
			}
		}

		if (minDist != (std::numeric_limits<float>::max)())
		{
			pointer->position = pos;
			focusedPos = pos;
			focusedObj = obj;
		}
	}

	void Pick()
	{
		if (focusedObj)
		{
			pickedObj = focusedObj;
			pickedPos = focusedPos;
		}
	}

	void UnPick()
	{
		pickedObj = nullptr;
		pickedPos = std::nullopt;
	}

	void MouseMoved(Vector2f mousePos, const Camera& camera)
	{
		this->mousePos = mousePos;
		auto vec4 = camera.proj.Inverse() * Vector4f(mousePos.x, mousePos.y, 1, 1);
		mouseDirection = Vector3f(vec4).Normalized();
		//pointer->position = camera.view.Inverse() * Vector4f(mouseDirection * 5, 1);

		if (pickedObj)
		{
			auto inverseView = camera.view.Inverse();
			Vector3f pickedPosView = camera.view * Vector4f(*pickedPos, 1);
			Vector3f newPos = inverseView * Vector4f(mouseDirection * pickedPosView.Length(), 1.);
			pickedObj->position += newPos - *pickedPos;
			*pickedPos = newPos;
		}
	}

	Vector2f mousePos;

	Vector3f mouseDirection;
	std::shared_ptr<MeshObject> pointer;

	std::shared_ptr<Object> focusedObj;
	std::optional<Vector3f> focusedPos;

	std::shared_ptr<Object> pickedObj;
	std::optional<Vector3f> pickedPos;
};
