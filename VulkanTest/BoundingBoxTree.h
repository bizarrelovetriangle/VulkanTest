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
				AddBoundingBoxObject(mesh.localBoundingBox, meshObject->ComposeMatrix());
				AddToTree(meshObject);
			}
		}
	}

	//GJK algorithm not required points rotation..
	void AddToTree(std::shared_ptr<MeshObject> meshObject)
	{
		auto& mesh = *meshObject->mesh;
		size_t newBoundingBox = boundingBoxes.size();
		boundingBoxes.emplace_back(mesh.localBoundingBox, meshObject->ComposeMatrix());
		boundingBoxes[newBoundingBox].sceneObject = meshObject;
		mesh.localBoundingBox.parent = newBoundingBox;
		AddBoundingBoxObject(boundingBoxes[newBoundingBox]);

		if (boundingBoxes.size() == 1) {
			rootBoundingBoxIndex = newBoundingBox;
			return;
		}

		int64_t neighbour = FindBestNeighbour(boundingBoxes[newBoundingBox]);

		size_t newParent = boundingBoxes.size();
		boundingBoxes.emplace_back(BoundingBox::Union(boundingBoxes[neighbour], boundingBoxes[newBoundingBox]));
		boundingBoxes[newParent].children.first = newBoundingBox;
		boundingBoxes[newParent].children.second = neighbour;
		boundingBoxes[newParent].parent = boundingBoxes[neighbour].parent;
		AddBoundingBoxObject(boundingBoxes[newParent]);

		boundingBoxes[newBoundingBox].parent = newParent;
		boundingBoxes[neighbour].parent = newParent;

		if (neighbour == rootBoundingBoxIndex) {
			rootBoundingBoxIndex = newParent;
			return;
		}

		int64_t& parentsChildIndex = boundingBoxes[boundingBoxes[newParent].parent].children.first == neighbour
			? boundingBoxes[boundingBoxes[newParent].parent].children.first
			: boundingBoxes[boundingBoxes[newParent].parent].children.second;
		parentsChildIndex = newParent;

		int64_t parent = boundingBoxes[newParent].parent;
		while (parent != -1) {
			auto& parentRef = boundingBoxes[parent];
			auto& childA = boundingBoxes[parentRef.children.first];
			auto& childB = boundingBoxes[parentRef.children.second];
			parentRef.Reset(childA, childB);
			AddBoundingBoxObject(parentRef);
			parent = parentRef.parent;
		}
	}

	void RemoveFromTree(std::shared_ptr<MeshObject> meshObject)
	{
	
	}

	int64_t FindBestNeighbour(BoundingBox& newBoundingBox)
	{
		using type = std::pair<float, int64_t>;
		auto less = [](type& a, type& b) { return a.first < b.first; };
		std::priority_queue<type, std::vector<type>, decltype(less)> pq(less);
		pq.push(std::make_pair(0., rootBoundingBoxIndex));
		type bestNeighbour = std::make_pair((std::numeric_limits<float>::max)(), -1);

		while (!pq.empty())
		{
			auto [prevExpantionVolume, index] = pq.top();
			pq.pop();

			auto& boundingBox = boundingBoxes[index];

			if (boundingBox.sceneObject)
			{
				float creationVolume = BoundingBox::Union(boundingBox, newBoundingBox).GetVolume();
				type potentialNeighbour = std::make_pair(creationVolume + prevExpantionVolume, index);
				bestNeighbour = (std::min<type>)(bestNeighbour, potentialNeighbour);
				continue;
			}

			float currentExpantionVolume = prevExpantionVolume + BoundingBox::Union(boundingBox, newBoundingBox).GetVolume() - boundingBox.GetVolume();

			if (currentExpantionVolume > bestNeighbour.first) continue;

			if (boundingBox.children.first != -1) pq.push(std::make_pair(currentExpantionVolume, boundingBox.children.first));
			if (boundingBox.children.second != -1) pq.push(std::make_pair(currentExpantionVolume, boundingBox.children.second));
		}

		return bestNeighbour.second;
	}

	void AddBoundingBoxObject(BoundingBox& boundingBox, Matrix4 model = Matrix4())
	{
		if (boundingBox.renderBoundingBoxObject) {
			boundingBox.renderBoundingBoxObject->Dispose();
			boundingBoxObjects.extract(boundingBox.renderBoundingBoxObject);
		}

		auto boundingBoxObject = std::make_shared<BoundingBoxObject>(vulkanContext, boundingBox);
		boundingBox.renderBoundingBoxObject = boundingBoxObject;
		auto& boundingBoxObjectModel = boundingBoxObject->renderer->transformUniform.model;
		boundingBoxObjectModel = model * boundingBoxObjectModel;
		boundingBoxObjects.insert(boundingBoxObject);
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

	size_t rootBoundingBoxIndex = 0;
	std::vector<BoundingBox> boundingBoxes;
	std::unordered_set<std::shared_ptr<BoundingBoxObject>> boundingBoxObjects;

private:
	VulkanContext& vulkanContext;
};