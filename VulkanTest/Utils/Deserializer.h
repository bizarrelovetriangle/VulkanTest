#pragma once
#include "../Objects/Object.h"

class Deserializer
{
public:
	Deserializer(VulkanContext& vulkanContext) : vulkanContext_(vulkanContext)
	{
	}

	std::unique_ptr<Object> Deserialize(SerializedObject& serializedObject)
	{
		std::unique_ptr<VertexedRenderObject> renderer;

		if (serializedObject.textureData.has_value())
		{
			renderer = std::make_unique<TexturedRenderObject>(vulkanContext_,
				*serializedObject.textureData, serializedObject.textureCoords);
		}
		else if (!serializedObject.colors.empty())
		{
			renderer = std::make_unique<ColoredRenderObject>(vulkanContext_, serializedObject.colors);
		}
		else if (!serializedObject.indexes.empty())
		{
			renderer = std::make_unique<SimpleVertexedRenderObject>(vulkanContext_);
		}

		renderer->propertiesUniform.baseColor = serializedObject.baseColor;
		renderer->UpdatePropertiesUniformBuffer();

		auto mesh = std::make_unique<MeshModel>(serializedObject.indexes, serializedObject.positions);

		auto object = std::make_unique<Object>(std::move(mesh), std::move(renderer));
		object->name = serializedObject.name;
		object->position = serializedObject.translation;
		object->rotation = serializedObject.rotation;
		object->scale = serializedObject.scale;

		object->renderer->transformUniform.model = object->ComposeMatrix();
		object->renderer->UpdateTransformUniformBuffer();
		object->renderer->UpdateVertexBuffer(*object->mesh);

		return std::move(object);
	}

private:
	VulkanContext& vulkanContext_;
};
