#pragma once
#include "Objects/Interfaces/Object.h"
#include "Objects/Interfaces/MeshObject.h"
#include "Camera.h"
#include "CAD/GeometryCreator.h"
#include "CAD/BoundingBoxTree.h"

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

	void Update(const BoundingBoxTree& boundingBoxTree)
	{
		auto viewToWorld = camera->worldToView.Inverse();
		auto line = std::make_pair(
			Vector3f(viewToWorld * Vector4f(Vector3f::Zero(), 1.)),
			Vector3f(viewToWorld * Vector4f(mouseDirection * 1000, 1.)));

		pointer->position = line.first;
		focusedPos = std::nullopt;
		focusedObj = nullptr;

		std::pair<float, Vector3f> nearestPos = { (std::numeric_limits<float>::max)(), {} };
		std::shared_ptr<Object> obj;

		std::vector<int64_t> boxes{ boundingBoxTree.rootBoundingBoxIndex };

		while (!boxes.empty())
		{
			auto currentBoxIndex = boxes.back();
			boxes.pop_back();

			if (currentBoxIndex == -1) {
				continue;
			}

			auto& currentBox = boundingBoxTree.boundingBoxes[currentBoxIndex];

			if (!currentBox.Intersect(line)) {
				continue;
			}

			if (currentBox.sceneObject) {
				if (!currentBox.sceneObject->interactive) continue;

				auto modelToWorld = currentBox.sceneObject->ComposeMatrix();
				auto worldToModel = modelToWorld.Inverse();

				auto modelSpaceLine = std::make_pair<Vector3f, Vector3f>(
					worldToModel * Vector4f(line.first, 1.),
					worldToModel * Vector4f(line.second, 1.));

				std::pair<float, Vector3f> pos;
				if (currentBox.sceneObject->mesh->Intersect(modelSpaceLine, &pos))
				{
					if (pos.first < nearestPos.first)
					{
						nearestPos = std::make_pair(pos.first, modelToWorld * Vector4f(pos.second, 1.));
						obj = currentBox.sceneObject;
					}
				}
			}
			else {
				boxes.push_back(currentBox.children.first);
				boxes.push_back(currentBox.children.second);
			}
		}

		if (nearestPos.first != (std::numeric_limits<float>::max)())
		{
			pointer->position = nearestPos.second;
			focusedPos = nearestPos.second;
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
