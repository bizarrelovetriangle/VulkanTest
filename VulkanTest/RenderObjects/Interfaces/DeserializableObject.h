#pragma once
#include "RenderObject.h"

class DeserializedObject;

class DeserializableObjectUniform
{
public:
	alignas(16) Vector4f baseColor;
	alignas(4) bool hasTexture = false;
	alignas(4) bool hasColors = false;
};

class DeserializableObject : public RenderObject
{
public:
	DeserializableObject(VulkanContext& vulkanContext, const DeserializedObject& deserializedObject);

	DeserializableObjectUniform uniform;
};
