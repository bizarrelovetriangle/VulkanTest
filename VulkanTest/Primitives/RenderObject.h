#pragma once
#include "../Math/Vector3.hpp"
#include "../Math/Vector2.hpp"
#include "../Math/Matrix4.hpp"
#include "../VulkanContext.h"
#include <optional>
#include <vulkan/vulkan.hpp>

class RenderVisitor;
class DescriptorSets;
struct DeserializedObject;
struct DeserializedObjectVertexData;
template <class T>
class BufferMemory;

class RenderObjectPushConstantRange
{
public:
	Matrix4 model;
	Matrix4 world;
};

class RenderObjectUniform
{
public:
	alignas(16) Vector4f baseColor;
	alignas(4) bool hasTexture = false;
	alignas(4) bool hasColors = false;
};

class RenderObject
{
public:
	RenderObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject);
	~RenderObject();
	virtual void Accept(RenderVisitor& renderVisitor) const;
	virtual void Dispose();

public:
	std::string name;
	Matrix4 model;

	RenderObjectUniform uniform;
	std::unique_ptr<BufferMemory<RenderObjectUniform>> uniformBuffer;

	std::unique_ptr<DescriptorSets> descriptorSets;
};

