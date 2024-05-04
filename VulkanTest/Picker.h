#pragma once
#include "Objects/Interfaces/Object.h"
#include "Objects/Interfaces/MeshObject.h"
#include "Camera.h"
#include "CAD/GeometryCreator.h"

class Picker
{
public:
	void Init(VulkanContext& vulkanContext, const Camera& camera)
	{
		this->camera = &camera;
		auto pointerRenderer = std::make_unique<SimpleVertexedRenderer>(vulkanContext);
		auto mesh = GeometryCreator::CreateIcosphere(0.1, 2);
		pointer = std::make_shared<MeshObject>(std::move(mesh), std::move(pointerRenderer));
		pointer->interactive = false;
	}

	void Update(const std::vector<std::shared_ptr<Object>>& objects)
	{
		auto viewToWorld = camera->worldToView.Inverse();
		auto segmentA = Vector3f(viewToWorld * Vector4f(Vector3f::Zero(), 1.));
		auto segmentB = Vector3f(viewToWorld * Vector4f(mouseDirection * 1000, 1.));

		pointer->position = segmentA;
		focusedPos = std::nullopt;
		focusedObj = nullptr;

		float minDist = (std::numeric_limits<float>::max)();
		Vector3f pos;
		std::shared_ptr<Object> obj;

		for (auto& object : objects)
		{
			if (!object->interactive) continue;

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

			Vector3f pickedPosView = camera->worldToView * Vector4f(*pickedPos, 1);
			pickedCameraDist = pickedPosView.Length();
		}
	}

	void UnPick()
	{
		pickedObj = nullptr;
		pickedPos = std::nullopt;
	}

	void MouseMoved(Vector2f mousePos)
	{
		this->mousePos = mousePos;
		auto projToView = camera->viewToProj.Inverse();
		auto vec4 = projToView * Vector4f(mousePos.x, mousePos.y, 1, 1);
		mouseDirection = Vector3f(vec4).Normalized();
		//pointer->position = camera->view.Inverse() * Vector4f(mouseDirection * 5, 1);
	}

	void UpdatePicked()
	{
		if (!pickedObj) return;

		auto viewToWorld = camera->worldToView.Inverse();
		Vector3f newPos = viewToWorld * Vector4f(mouseDirection * pickedCameraDist, 1.);
		pickedObj->position += newPos - *pickedPos;
		*pickedPos = newPos;
	}

	Vector2f mousePos;

	Vector3f mouseDirection;
	std::shared_ptr<MeshObject> pointer;

	std::shared_ptr<Object> focusedObj;
	std::optional<Vector3f> focusedPos;

	std::shared_ptr<Object> pickedObj;
	std::optional<Vector3f> pickedPos;
	float pickedCameraDist = 0.;

private:
	const Camera* camera;
};
