#pragma once
#include "../Objects/Interfaces/MeshObject.h"
#include "../Renderers/LinedRenderer.h"

#include "Renderers/ColoredRenderer.h"
#include "Renderers/TexturedRenderer.h"
#include "Renderers/SimpleVertexedRenderer.h"
#include "Renderers/PlaneRenderer.h"

class Deserializer
{
public:
	Deserializer(VulkanContext& vulkanContext) : vulkanContext_(vulkanContext)
	{
	}

	std::unique_ptr<MeshObject> Deserialize(SerializedObject& serializedObject)
	{
		std::unique_ptr<VertexedRenderer> renderer;

		if (serializedObject.textureData.has_value())
		{
			renderer = std::make_unique<TexturedRenderer>(vulkanContext_,
				*serializedObject.textureData, serializedObject.textureCoords);
		}
		else if (!serializedObject.colors.empty())
		{
			renderer = std::make_unique<ColoredRenderer>(vulkanContext_, serializedObject.colors);
		}
		else if (!serializedObject.indexes.empty())
		{
			renderer = std::make_unique<SimpleVertexedRenderer>(vulkanContext_);
		}

		renderer->propertiesUniform.baseColor = serializedObject.baseColor;
		renderer->UpdatePropertiesUniformBuffer();

		auto mesh = std::make_unique<MeshModel>(serializedObject.indexes, serializedObject.positions);

		auto object = std::make_unique<MeshObject>(std::move(mesh), std::move(renderer));
		object->name = serializedObject.name;
		object->position = serializedObject.translation;
		object->rotation = serializedObject.rotation;
		object->scale = serializedObject.scale;

		return std::move(object);
	}

private:
	VulkanContext& vulkanContext_;
};
