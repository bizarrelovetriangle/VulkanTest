#pragma once
#include "Objects/Primitives/BoundingBoxObject.h"
#include "RenderVisitor.h"
#include "CAD/BoundingBox.h"
#include <queue>

class BoundingBoxTree : public Object
{
public:
	BoundingBoxTree(VulkanContext& vulkanContext)
		: vulkanContext(vulkanContext)
	{

	}
	//local boxes for all convex parts of mesh
	//boxes for local boxes
	//Global unlocal tree is created here
	void CreateBoundingBoxes(std::vector<std::shared_ptr<Object>>& objects)
	{
		for (auto& object : objects)
		{
			if (auto meshObject = std::dynamic_pointer_cast<MeshObject>(object); meshObject)
			{
				auto& mesh = *meshObject->mesh;

				{
					auto& renderBoundingBoxObject = mesh.localBoundingBox.renderBoundingBoxObject;
					renderBoundingBoxObject = std::make_shared<BoundingBoxObject>(vulkanContext, mesh.localBoundingBox);
					auto& model = renderBoundingBoxObject->renderer->transformUniform.model;
					model = meshObject->ComposeMatrix() * model;
				}

				AddToTree(meshObject);
			}
		}
	}

	void AddToTree(std::shared_ptr<MeshObject> meshObject)
	{
		auto& mesh = *meshObject->mesh;
		auto boundingBox = std::make_shared<BoundingBox>(mesh.localBoundingBox, meshObject->ComposeMatrix());
		boundingBox->renderBoundingBoxObject = std::make_shared<BoundingBoxObject>(vulkanContext, *boundingBox);
		boundingBox->sceneObject = meshObject;

		if (!rootBoundingBox) {
			rootBoundingBox = boundingBox;
			return;
		}

		auto temp = std::make_shared<BoundingBox>(BoundingBox::Union(*rootBoundingBox, *boundingBox));
		temp->renderBoundingBoxObject = std::make_shared<BoundingBoxObject>(vulkanContext, *temp);
		temp->children[0] = boundingBox;
		temp->children[1] = rootBoundingBox;

		rootBoundingBox = temp;
	}

	virtual void Render(RenderVisitor& renderVisitor) override
	{
		std::queue<std::shared_ptr<BoundingBox>> q;
		q.push(rootBoundingBox);

		while (!q.empty())
		{
			auto& box = q.front();
			if (box->renderBoundingBoxObject)
				box->renderBoundingBoxObject->Render(renderVisitor);

			if (box->sceneObject)
				box->sceneObject->mesh->localBoundingBox.renderBoundingBoxObject->Render(renderVisitor);

			if (box->children[0]) q.push(box->children[0]);
			if (box->children[1]) q.push(box->children[1]);
			q.pop();
		}
	}

	virtual void Dispose() override
	{
		std::queue<std::shared_ptr<BoundingBox>> q;
		q.push(rootBoundingBox);

		while (!q.empty())
		{
			auto& box = q.front();
			if (box->renderBoundingBoxObject)
				box->renderBoundingBoxObject->Dispose();

			if (box->sceneObject)
				box->sceneObject->mesh->localBoundingBox.renderBoundingBoxObject->Dispose();

			if (box->children[0]) q.push(box->children[0]);
			if (box->children[1]) q.push(box->children[1]);
			q.pop();
		}
	}

	std::shared_ptr<BoundingBox> rootBoundingBox;

private:
	VulkanContext& vulkanContext;
};