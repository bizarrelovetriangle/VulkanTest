#pragma once
#include "../Objects/Primitives/BoundingBoxObject.h"
#include "../RenderVisitor.h"
#include "BoundingBox.h"
#include "MeshContactAlgorithms.h"
#include <queue>
#include <set>
#include <unordered_set>

class BoundingBoxTree : public Object
{
public:
	BoundingBoxTree(VulkanContext& vulkanContext)
		: vulkanContext(vulkanContext), meshContactAlgorithms(vulkanContext)
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

	std::vector<ContactInfo> ComposePairs()
	{
		auto pair = std::make_pair(boundingBoxes[0].sceneObject, boundingBoxes[1].sceneObject);

		auto contact = meshContactAlgorithms.CheckContact(pair.first, pair.second);

		return { contact };
	}

	// EPA
	// Update: GJK required to proceed through all points anyway
	// 
	// UpdateVertexBuffer - optimize by just copying buffers with no interleafing. Triangle normals
	// But what about deleted triangles?..
	// 
	// Utilize Vulkan memory barriers
	// 
	// GJK algorithm not required points rotation.. Or not
	// Shall we do what now. GJP for two convex bodies, look that out, test that out
	void AddToTree(std::shared_ptr<MeshObject> meshObject)
	{
		auto& mesh = *meshObject->mesh;
		size_t newBoundingBox = NextFree();
		boundingBoxes[newBoundingBox] = BoundingBox(mesh.localBoundingBox, meshObject->ComposeMatrix());
		boundingBoxes[newBoundingBox].sceneObject = meshObject;
		mesh.localBoundingBox.parent = newBoundingBox;
		AddBoundingBoxObject(boundingBoxes[newBoundingBox]);

		if (boundingBoxes.size() == 1) {
			rootBoundingBoxIndex = newBoundingBox;
			return;
		}

		int64_t neighbour = FindBestNeighbour(boundingBoxes[newBoundingBox]);

		size_t newParent = NextFree();
		boundingBoxes[newParent] = BoundingBox::Union(boundingBoxes[neighbour], boundingBoxes[newBoundingBox]);
		boundingBoxes[newParent].children.first = newBoundingBox;
		boundingBoxes[newParent].children.second = neighbour;
		boundingBoxes[newParent].parent = boundingBoxes[neighbour].parent;
		AddBoundingBoxObject(boundingBoxes[newParent]);

		if (neighbour != rootBoundingBoxIndex) {
			ParentRef(neighbour) = newParent;
		}

		boundingBoxes[newBoundingBox].parent = newParent;
		boundingBoxes[neighbour].parent = newParent;

		if (neighbour == rootBoundingBoxIndex) {
			rootBoundingBoxIndex = newParent;
			return;
		}

		int64_t parent = boundingBoxes[newParent].parent;
		while (parent != -1) {
			auto& parentRef = boundingBoxes[parent];
			parentRef.Reset(boundingBoxes[parentRef.children.first], boundingBoxes[parentRef.children.second]);
			AddBoundingBoxObject(parentRef);
			parent = parentRef.parent;
		}
	}

	void RemoveFromTree(std::shared_ptr<MeshObject> meshObject)
	{
		int64_t boundingBox = meshObject->mesh->localBoundingBox.parent;

		auto& object = boundingBoxes[boundingBox].renderBoundingBoxObject;
		object->Dispose();
		boundingBoxObjects.erase(object);
		freeBuckets.push_back(boundingBox);

		if (boundingBox == rootBoundingBoxIndex) {
			rootBoundingBoxIndex = -1;
			return;
		}

		int64_t parentBoundingBox = boundingBoxes[boundingBox].parent;
		int64_t siblingBoundingBox = Sibling(boundingBox);
		boundingBoxes[siblingBoundingBox].parent = boundingBoxes[parentBoundingBox].parent;
		ParentRef(parentBoundingBox) = siblingBoundingBox;

		auto& parentObject = boundingBoxes[parentBoundingBox].renderBoundingBoxObject;
		parentObject->Dispose();
		boundingBoxObjects.erase(parentObject);
		freeBuckets.push_back(parentBoundingBox);

		int64_t parent = boundingBoxes[siblingBoundingBox].parent;
		while (parent != -1) {
			auto& parentRef = boundingBoxes[parent];
			parentRef.Reset(boundingBoxes[parentRef.children.first], boundingBoxes[parentRef.children.second]);
			AddBoundingBoxObject(parentRef);
			parent = parentRef.parent;
		}
	}

	int64_t FindBestNeighbour(BoundingBox& newBoundingBox)
	{
		using type = std::pair<float, int64_t>;
		auto less = [](type& a, type& b) { return a.first > b.first; };
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
				bestNeighbour = (std::min)(bestNeighbour, potentialNeighbour);
				continue;
			}

			float currentExpantionVolume = prevExpantionVolume + BoundingBox::Union(boundingBox, newBoundingBox).GetVolume() - boundingBox.GetVolume();

			if (currentExpantionVolume > bestNeighbour.first) continue;

			pq.push(std::make_pair(currentExpantionVolume, boundingBox.children.first));
			pq.push(std::make_pair(currentExpantionVolume, boundingBox.children.second));
		}

		return bestNeighbour.second;
	}

	void AddBoundingBoxObject(BoundingBox& boundingBox, Matrix4 model = Matrix4())
	{
		if (boundingBox.renderBoundingBoxObject) {
			boundingBox.renderBoundingBoxObject->Dispose();
			boundingBoxObjects.erase(boundingBox.renderBoundingBoxObject);
		}

		auto boundingBoxObject = std::make_shared<BoundingBoxObject>(vulkanContext, boundingBox);
		boundingBox.renderBoundingBoxObject = boundingBoxObject;
		auto& boundingBoxObjectModel = boundingBoxObject->renderer->transformUniform.model;
		boundingBoxObjectModel = model * boundingBoxObjectModel;
		boundingBoxObjects.insert(boundingBoxObject);
	}

	int64_t& ParentRef(int64_t boundingBox)
	{
		auto parent = boundingBoxes[boundingBox].parent;
		return boundingBoxes[parent].children.first == boundingBox
			? boundingBoxes[parent].children.first
			: boundingBoxes[parent].children.second;
	}

	int64_t& Sibling(int64_t boundingBox)
	{
		auto parent = boundingBoxes[boundingBox].parent;
		return boundingBoxes[parent].children.first == boundingBox
			? boundingBoxes[parent].children.second
			: boundingBoxes[parent].children.first;
	}

	int64_t NextFree()
	{
		if (!freeBuckets.empty()) {
			int64_t next = freeBuckets.back();
			freeBuckets.pop_back();
			return next;
		}

		int64_t next = boundingBoxes.size();
		boundingBoxes.resize(boundingBoxes.size() + 1);
		return next;
	}

	virtual void Render(RenderVisitor& renderVisitor, const Camera& camera) override
	{
		//for (auto& boundingBoxObject : boundingBoxObjects)
		//	boundingBoxObject->Render(renderVisitor, camera);
		meshContactAlgorithms.Render(renderVisitor, camera);
	}

	virtual void Dispose() override
	{
		for (auto& boundingBoxObject : boundingBoxObjects)
			boundingBoxObject->Dispose();
		meshContactAlgorithms.Dispose();
	}

	int64_t rootBoundingBoxIndex = 0;
	std::vector<int64_t> freeBuckets;
	std::vector<BoundingBox> boundingBoxes;
	std::unordered_set<std::shared_ptr<BoundingBoxObject>> boundingBoxObjects;

private:
	VulkanContext& vulkanContext;
	MeshContactAlgorithms meshContactAlgorithms;
};