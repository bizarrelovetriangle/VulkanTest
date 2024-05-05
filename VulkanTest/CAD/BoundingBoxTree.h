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

	void UpdateTree(std::vector<std::shared_ptr<Object>>& objects)
	{
		for (auto& object : objects)
		{
			if (!object->interactive) continue;

			if (auto meshObject = std::dynamic_pointer_cast<MeshObject>(object); meshObject)
			{
				UpdateBoundingBoxObject(meshObject->mesh->localBoundingBox, meshObject);

				if (meshObject->mesh->localBoundingBox.parent == -1) {
					AddToTree(meshObject);
				}
				else {
					auto& orgBoundingBox = boundingBoxes[meshObject->mesh->localBoundingBox.parent];
					auto boundingBox = BoundingBox(meshObject->mesh->localBoundingBox, meshObject->ComposeMatrix());

					if (boundingBox.Exceed(orgBoundingBox)) {
						RemoveFromTree(meshObject);
						AddToTree(meshObject);
					}
				}
			}
		}
	}

	std::vector<ContactInfo> ComposePairs()
	{
		auto pair = std::make_pair(boundingBoxes[0].sceneObject, boundingBoxes[1].sceneObject);

		auto contact = meshContactAlgorithms.CheckContact(pair.first, pair.second);

		return { contact };
	}

	// Create uniform class
	// Fix BoundingBoxTree. Make it update objects in tree while they move
	// BoundingBoxTree merge tree nodes and check collisions
	// Test the Picker with BoundingBoxes
	// 
	// UpdateVertexBuffer - optimize by just copying buffers with no interleafing. 
	// How to deal with triangle normals. Geometrical shader
	// But what about deleted triangles?..
	// Interactive vector class with arrows
	// Edge and Edge Data
	// 
	// Utilize Vulkan memory barriers

	void AddToTree(std::shared_ptr<MeshObject> meshObject)
	{
		auto& mesh = *meshObject->mesh;
		size_t newBoundingBox = NextFree();
		boundingBoxes[newBoundingBox] = BoundingBox(mesh.localBoundingBox, meshObject->ComposeMatrix(), 0.3);
		boundingBoxes[newBoundingBox].sceneObject = meshObject;
		mesh.localBoundingBox.parent = newBoundingBox;
		UpdateBoundingBoxObject(boundingBoxes[newBoundingBox]);

		if (rootBoundingBoxIndex == -1) {
			rootBoundingBoxIndex = newBoundingBox;
			return;
		}

		int64_t sibling = FindBestSibling(boundingBoxes[newBoundingBox]);

		size_t newParent = NextFree();
		boundingBoxes[newParent] = BoundingBox::Union(boundingBoxes[sibling], boundingBoxes[newBoundingBox]);
		boundingBoxes[newParent].children.first = newBoundingBox;
		boundingBoxes[newParent].children.second = sibling;
		boundingBoxes[newParent].parent = boundingBoxes[sibling].parent;
		UpdateBoundingBoxObject(boundingBoxes[newParent]);

		if (sibling == rootBoundingBoxIndex) {
			rootBoundingBoxIndex = newParent;
		}
		else {
			ParentsRefToThis(sibling) = newParent;
		}

		boundingBoxes[newBoundingBox].parent = newParent;
		boundingBoxes[sibling].parent = newParent;

		int64_t parent = boundingBoxes[newParent].parent;
		while (parent != -1) {
			auto& parentRef = boundingBoxes[parent];
			parentRef.Reset(boundingBoxes[parentRef.children.first], boundingBoxes[parentRef.children.second]);
			UpdateBoundingBoxObject(parentRef);
			parent = parentRef.parent;
		}
	}

	void RemoveFromTree(std::shared_ptr<MeshObject> meshObject)
	{
		RemoveBoundingBoxObjects(meshObject);

		int64_t boundingBox = meshObject->mesh->localBoundingBox.parent;
		meshObject->mesh->localBoundingBox.parent = -1;
		freeBuckets.push_back(boundingBox);

		if (boundingBox == rootBoundingBoxIndex) {
			rootBoundingBoxIndex = -1;
			return;
		}

		int64_t parentBoundingBox = boundingBoxes[boundingBox].parent;
		int64_t siblingBoundingBox = Sibling(boundingBox);
		boundingBoxes[siblingBoundingBox].parent = boundingBoxes[parentBoundingBox].parent;

		if (parentBoundingBox == rootBoundingBoxIndex) {
			rootBoundingBoxIndex = siblingBoundingBox;
		}
		else {
			ParentsRefToThis(parentBoundingBox) = siblingBoundingBox;
		}

		auto& parentObject = boundingBoxes[parentBoundingBox].renderBoundingBoxObject;
		parentObject->Dispose();
		boundingBoxObjects.erase(parentObject);
		freeBuckets.push_back(parentBoundingBox);

		int64_t parent = boundingBoxes[siblingBoundingBox].parent;
		while (parent != -1) {
			auto& parentRef = boundingBoxes[parent];
			parentRef.Reset(boundingBoxes[parentRef.children.first], boundingBoxes[parentRef.children.second]);
			UpdateBoundingBoxObject(parentRef);
			parent = parentRef.parent;
		}
	}

	int64_t FindBestSibling(BoundingBox& newBoundingBox)
	{
		using type = std::pair<float, int64_t>;
		auto less = [](type& a, type& b) { return a.first > b.first; };
		std::priority_queue<type, std::vector<type>, decltype(less)> pq(less);
		pq.push(std::make_pair(0., rootBoundingBoxIndex));
		type bestSibling = std::make_pair((std::numeric_limits<float>::max)(), -1);

		while (!pq.empty())
		{
			auto [prevExpantionVolume, index] = pq.top();
			pq.pop();

			auto& potentSiblingBox = boundingBoxes[index];

			float unionBoxVolume = BoundingBox::Union(potentSiblingBox, newBoundingBox).GetVolume();
			float volumeToStayHere = prevExpantionVolume + unionBoxVolume;
			type potentialSibling = std::make_pair(volumeToStayHere, index);
			bestSibling = (std::min)(bestSibling, potentialSibling);

			if (potentSiblingBox.sceneObject)
			{
				continue;
			}

			float currentExpantionVolume = volumeToStayHere - potentSiblingBox.GetVolume();

			if (currentExpantionVolume > bestSibling.first) continue;

			pq.push(std::make_pair(currentExpantionVolume, potentSiblingBox.children.first));
			pq.push(std::make_pair(currentExpantionVolume, potentSiblingBox.children.second));
		}

		return bestSibling.second;
	}

	int64_t& ParentsRefToThis(int64_t boundingBox)
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

	void UpdateBoundingBoxObject(BoundingBox& boundingBox, std::shared_ptr<MeshObject> refObject = nullptr)
	{
		auto& boundingBoxObject = boundingBox.renderBoundingBoxObject;

		if (!boundingBoxObject) {
			boundingBoxObject = std::make_shared<BoundingBoxObject>(vulkanContext, boundingBox);
			boundingBoxObjects.insert(boundingBoxObject);

			if (refObject) {
				boundingBoxObject->renderer->propertiesUniform.baseColor = Vector4f(1., 1., 1., 1.);
				boundingBoxObject->renderer->UpdatePropertiesUniformBuffer();
			}
		}
		else {
			boundingBoxObject->UpdateBoundingBox(boundingBox);
		}

		if (refObject) {
			boundingBoxObject->position = refObject->position;
			boundingBoxObject->rotation = refObject->rotation;
			boundingBoxObject->scale = refObject->scale;
		}
	}

	void RemoveBoundingBoxObjects(std::shared_ptr<MeshObject> meshObject)
	{
		int64_t boundingBox = meshObject->mesh->localBoundingBox.parent;
		auto& object = boundingBoxes[boundingBox].renderBoundingBoxObject;
		object->Dispose();
		boundingBoxObjects.erase(object);
	}

	virtual void Render(RenderVisitor& renderVisitor) override
	{
		for (auto& boundingBoxObject : boundingBoxObjects)
			boundingBoxObject->Render(renderVisitor);
		meshContactAlgorithms.Render(renderVisitor);
	}

	virtual void Dispose() override
	{
		for (auto& boundingBoxObject : boundingBoxObjects)
			boundingBoxObject->Dispose();
		boundingBoxes.clear();
		boundingBoxObjects.clear();
		rootBoundingBoxIndex = -1;

		meshContactAlgorithms.Dispose();
	}

	int64_t rootBoundingBoxIndex = -1;
	std::vector<int64_t> freeBuckets;
	std::vector<BoundingBox> boundingBoxes;
	std::unordered_set<std::shared_ptr<BoundingBoxObject>> boundingBoxObjects;

private:
	VulkanContext& vulkanContext;
	MeshContactAlgorithms meshContactAlgorithms;
};