#pragma once
#include "VertexedRenderObject.h"

class DeserializedObject;

class DeserializableObject : public VertexedRenderObject
{
public:
	DeserializableObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject);
	~DeserializableObject();
	virtual void Dispose() override;
	static std::vector<vk::DescriptorSetLayoutBinding> DescriptorSetLayoutBinding();

public:
	std::string gltfName;
};
