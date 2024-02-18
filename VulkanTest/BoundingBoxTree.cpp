#pragma once
#include "Objects/Primitives/BoundingBoxObject.h"
#include "RenderVisitor.h"
#include "CAD/BoundingBox.h"

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
					boundingBoxes.push_back(mesh.localBoundingBox);
				}

				BoundingBox boundingBox(mesh.localBoundingBox, meshObject->ComposeMatrix());
				boundingBox.renderBoundingBoxObject = std::make_shared<BoundingBoxObject>(vulkanContext, boundingBox);
				boundingBoxes.push_back(boundingBox);
			}
		}
	}

	virtual void Render(RenderVisitor& renderVisitor) override
	{
		for (auto& object : boundingBoxes)
		{
			if (object.renderBoundingBoxObject)
				object.renderBoundingBoxObject->Render(renderVisitor);
		}
	}

	virtual void Dispose() override
	{
		for (auto& object : boundingBoxes)
			if (object.renderBoundingBoxObject)
				object.renderBoundingBoxObject->Dispose();
	}

	std::vector<BoundingBox> boundingBoxes;

private:
	VulkanContext& vulkanContext;
};