#pragma once
#include "../Objects/Primitives/BoundingBoxObject.h"
#include "../RenderVisitor.h"
#include "BoundingBox.h"
#include "MeshContactAlgorithms.h"
#include <queue>
#include <set>
#include <unordered_set>

namespace
{
	const bool visibleBoundingBoxes = false;
};

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

			if (auto meshObject = std::dynamic_pointer_cast<MeshObject>(object); meshObject && !meshObject->convexSegments.empty())
			{
				for (auto& mesh : meshObject->convexSegments) {
					UpdateBoundingBoxObject(mesh->localBoundingBox, meshObject);

					if (mesh->localBoundingBox.parent == -1) {
						AddToTree(mesh, meshObject);
					}
					else {
						auto& orgBoundingBox = boundingBoxes[mesh->localBoundingBox.parent];
						auto boundingBox = BoundingBox(mesh->localBoundingBox, meshObject->ComposeMatrix());

						if (boundingBox.Exceed(orgBoundingBox)) {
							RemoveFromTree(mesh, meshObject);
							AddToTree(mesh, meshObject);
						}
					}
				}
			}
		}
	}

	std::vector<ContactInfo> ComposePairs()
	{
		meshContactAlgorithms.Dispose();

		std::vector<ContactInfo> result;
		if (rootBoundingBoxIndex == -1) return result;

		struct TraverseInfo {
			std::pair<int64_t, int64_t> pair;
			bool selfCheck;
		};

		std::vector<TraverseInfo> traverseQueue{ { boundingBoxes[rootBoundingBoxIndex].children, true } };
		std::set<std::pair<std::shared_ptr<MeshObject>, std::shared_ptr<MeshObject>>> collidedObjects;

		while (!traverseQueue.empty())
		{
			auto traverseInfo = traverseQueue.back();
			traverseQueue.pop_back();
			auto& [first, second] = traverseInfo.pair;

			if (first != -1 && second != -1 && boundingBoxes[first].Intersect(boundingBoxes[second])) {
				if (boundingBoxes[first].sceneObject && boundingBoxes[second].sceneObject) {
					if (boundingBoxes[first].sceneObject != boundingBoxes[second].sceneObject) {
						bool collided = collidedObjects.contains(
							std::make_pair(boundingBoxes[first].sceneObject, boundingBoxes[second].sceneObject));

						if (!collided) {
							auto contactInfo = meshContactAlgorithms.CheckContact(
								boundingBoxes[first].sceneObject, boundingBoxes[second].sceneObject,
								boundingBoxes[first].sceneMesh, boundingBoxes[second].sceneMesh);

							if (contactInfo.contact) {
								// There is no typo. We are adding the symmetry
								collidedObjects.emplace(boundingBoxes[first].sceneObject, boundingBoxes[second].sceneObject);
								collidedObjects.emplace(boundingBoxes[second].sceneObject, boundingBoxes[first].sceneObject);

								result.push_back(contactInfo);
							}
						}
					}

					continue;
				}

				if (boundingBoxes[first].sceneObject) {
					traverseQueue.emplace_back(std::make_pair(first, boundingBoxes[second].children.first), false);
					traverseQueue.emplace_back(std::make_pair(first, boundingBoxes[second].children.second), false);
				}
				else if (boundingBoxes[second].sceneObject) {
					traverseQueue.emplace_back(std::make_pair(second, boundingBoxes[first].children.first), false);
					traverseQueue.emplace_back(std::make_pair(second, boundingBoxes[first].children.second), false);
				}
				else {
					traverseQueue.emplace_back(std::make_pair(boundingBoxes[first].children.first, boundingBoxes[second].children.first), false);
					traverseQueue.emplace_back(std::make_pair(boundingBoxes[first].children.first, boundingBoxes[second].children.second), false);

					traverseQueue.emplace_back(std::make_pair(boundingBoxes[first].children.second, boundingBoxes[second].children.first), false);
					traverseQueue.emplace_back(std::make_pair(boundingBoxes[first].children.second, boundingBoxes[second].children.second), false);
				}
			}

			if (traverseInfo.selfCheck) {
				if (first != -1 && !boundingBoxes[first].sceneObject) {
					traverseQueue.emplace_back(boundingBoxes[first].children, true);
				}
				if (second != -1 && !boundingBoxes[second].sceneObject) {
					traverseQueue.emplace_back(boundingBoxes[second].children, true);
				}
			}
		}

		return result;
	}

	// BindingSets allocation
	// Fix BoundingBox::Intersect
	// Test the Picker with BoundingBoxes
	// BoundingBoxTree merge tree nodes and check collisions
	// TextureObjext normal map
	// 
	// UpdateVertexBuffer - optimize by just copying buffers with no interleafing. 
	// How to deal with triangle normals. Geometrical shader
	// But what about deleted triangles?..
	// Interactive vector class with arrows
	// Edge and Edge Data
	// 
	// Utilize Vulkan memory barriers

	// backlog
	// Add one element BufferData::Flush override
	// Create uniform class

	void AddToTree(std::shared_ptr<MeshModel>& mesh, std::shared_ptr<MeshObject>& object)
	{
		size_t newBoundingBox = NextFree();
		auto prevRender = boundingBoxes[newBoundingBox].renderBoundingBoxObject;
		boundingBoxes[newBoundingBox] = BoundingBox(mesh->localBoundingBox, object->ComposeMatrix(), 0.3);
		boundingBoxes[newBoundingBox].renderBoundingBoxObject = prevRender;
		boundingBoxes[newBoundingBox].sceneObject = object;
		boundingBoxes[newBoundingBox].sceneMesh = mesh;
		mesh->localBoundingBox.parent = newBoundingBox;
		UpdateBoundingBoxObject(boundingBoxes[newBoundingBox]);

		if (rootBoundingBoxIndex == -1) {
			rootBoundingBoxIndex = newBoundingBox;
			return;
		}

		int64_t sibling = FindBestSibling(boundingBoxes[newBoundingBox]);

		size_t newParent = NextFree();
		auto prevUnionRender = boundingBoxes[newBoundingBox].renderBoundingBoxObject;
		boundingBoxes[newParent] = BoundingBox::Union(boundingBoxes[sibling], boundingBoxes[newBoundingBox]);
		boundingBoxes[newParent].renderBoundingBoxObject = prevUnionRender;
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

	void RemoveFromTree(std::shared_ptr<MeshModel>& mesh, std::shared_ptr<MeshObject>& object)
	{
		int64_t boundingBox = mesh->localBoundingBox.parent;
		RemoveBoundingBoxObjects(boundingBoxes[boundingBox]);
		mesh->localBoundingBox.parent = -1;
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

		RemoveBoundingBoxObjects(boundingBoxes[parentBoundingBox]);
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
		std::priority_queue<type, std::vector<type>, std::greater<type>> pq;
		pq.push(std::make_pair(0., rootBoundingBoxIndex));
		type bestSibling = std::make_pair((std::numeric_limits<float>::max)(), -1);

		while (!pq.empty())
		{
			auto [prevExpantionVolume, index] = pq.top();
			pq.pop();

			auto& potentSiblingBox = boundingBoxes[index];

			float unionBoxVolume = BoundingBox::Union(potentSiblingBox, newBoundingBox).GetVolume();
			float volumeToStayHere = prevExpantionVolume + unionBoxVolume;

			if (volumeToStayHere < bestSibling.first) {
				bestSibling = std::make_pair(volumeToStayHere, index);
			}

			if (potentSiblingBox.sceneObject)
			{
				continue;
			}

			float currentExpantionVolume = volumeToStayHere - potentSiblingBox.GetVolume();

			if (currentExpantionVolume > bestSibling.first) continue;

			pq.emplace(currentExpantionVolume, potentSiblingBox.children.first);
			pq.emplace(currentExpantionVolume, potentSiblingBox.children.second);
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
		if (!visibleBoundingBoxes) {
			return;
		}

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
			boundingBoxObject->visible = true;
			boundingBoxObject->UpdateBoundingBox(boundingBox);
		}

		if (refObject) {
			boundingBoxObject->position = refObject->position;
			boundingBoxObject->rotation = refObject->rotation;
			boundingBoxObject->scale = refObject->scale;
		}
	}

	void RemoveBoundingBoxObjects(BoundingBox& boundingBox)
	{
		if (!visibleBoundingBoxes) {
			return;
		}

		auto& object = boundingBox.renderBoundingBoxObject;
		object->visible = false;
		//object->Dispose();
		//boundingBoxObjects.erase(object);
	}

	virtual void Render(RenderVisitor& renderVisitor) override
	{
		for (auto& boundingBoxObject : boundingBoxObjects)
		{
			if (boundingBoxObject->visible) {
			boundingBoxObject->Render(renderVisitor);
			}
		}
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