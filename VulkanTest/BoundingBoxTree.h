#pragma once
#include "Objects/Primitives/BoundingBoxObject.h"
#include "RenderVisitor.h"
#include "CAD/BoundingBox.h"
#include <queue>
#include <set>
#include <unordered_set>

class BoundingBoxTree : public Object
{
public:
	BoundingBoxTree(VulkanContext& vulkanContext)
		: vulkanContext(vulkanContext)
	{

	}

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
					boundingBoxObjects.insert(renderBoundingBoxObject);
				}

				AddToTree(meshObject);
			}
		}
	}

	void AddToTree(std::shared_ptr<MeshObject> meshObject)
	{
		auto& mesh = *meshObject->mesh;
		size_t newBoundingBoxIndex = boundingBoxes.size();
		BoundingBox& newBoundingBox = boundingBoxes.emplace_back(mesh.localBoundingBox, meshObject->ComposeMatrix());
		newBoundingBox.renderBoundingBoxObject = std::make_shared<BoundingBoxObject>(vulkanContext, newBoundingBox);
		boundingBoxObjects.insert(newBoundingBox.renderBoundingBoxObject);
		newBoundingBox.sceneObject = meshObject;

		if (boundingBoxes.size() == 1) {
			rootBoundingBox = newBoundingBoxIndex;
			return;
		}

		size_t tempBoundingBoxIndex = boundingBoxes.size();
		BoundingBox& temp = boundingBoxes.emplace_back(BoundingBox::Union(boundingBoxes[rootBoundingBox], newBoundingBox));
		temp.renderBoundingBoxObject = std::make_shared<BoundingBoxObject>(vulkanContext, temp);
		boundingBoxObjects.insert(temp.renderBoundingBoxObject);
		temp.children[0] = newBoundingBoxIndex;
		temp.children[1] = rootBoundingBox;
		rootBoundingBox = tempBoundingBoxIndex;
	}

	void AddToTree(std::shared_ptr<MeshObject> meshObject)
	{
	
	}

	virtual void Render(RenderVisitor& renderVisitor) override
	{
		for (auto& boundingBoxObject : boundingBoxObjects)
			boundingBoxObject->Render(renderVisitor);
	}

	virtual void Dispose() override
	{
		for (auto& boundingBoxObject : boundingBoxObjects)
			boundingBoxObject->Dispose();
	}

	size_t rootBoundingBox = 0;
	std::vector<BoundingBox> boundingBoxes;
	std::unordered_set<std::shared_ptr<BoundingBoxObject>> boundingBoxObjects;

private:
	VulkanContext& vulkanContext;
};