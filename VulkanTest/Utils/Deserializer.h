#pragma once
#include "../Objects/Interfaces/Object.h"
#include "../RenderObjects/LinedRenderObject.h"

class Deserializer
{
public:
	Deserializer(VulkanContext& vulkanContext) : vulkanContext_(vulkanContext)
	{
	}

	std::unique_ptr<MeshObject> Deserialize(SerializedObject& serializedObject)
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
			renderer = std::make_unique<LinedRenderObject>(vulkanContext_);
		}

		renderer->propertiesUniform.baseColor = serializedObject.baseColor;
		renderer->UpdatePropertiesUniformBuffer();

		auto mesh = std::make_unique<MeshModel>(serializedObject.indexes, serializedObject.positions);

		auto object = std::make_unique<MeshObject>(std::move(mesh), std::move(renderer));
		object->name = serializedObject.name;
		object->position = serializedObject.translation;
		object->rotation = serializedObject.rotation;
		object->scale = serializedObject.scale;

		object->renderer->transformUniform.model = object->ComposeMatrix();
		object->renderer->UpdateTransformUniformBuffer();
		object->UpdateVertexBuffer();

		return std::move(object);
	}

private:
	VulkanContext& vulkanContext_;
};
