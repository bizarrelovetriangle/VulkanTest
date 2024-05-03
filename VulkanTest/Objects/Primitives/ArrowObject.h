#pragma once
#include "../Interfaces/MeshObject.h"
#include "../../Math/Plane.h"
#include "../../Renderers/LinedRenderer.h"

class ArrowObject : public MeshObject
{
public:
	ArrowObject(VulkanContext& vulkanContext, const Vector3f& position, const Vector3f& direction)
	{
		this->position = position;
		this->direction = direction;

		float length = 1.;
		float tipSize = 0.1;
		std::vector<Vector3f> positions{ {0., 0., 0.}, {0., length, 0.},
			{-tipSize, length - tipSize, -tipSize},
			{ tipSize, length - tipSize, -tipSize},
			{ tipSize, length - tipSize,  tipSize},
			{-tipSize, length - tipSize,  tipSize}, };
		std::vector<uint32_t> indexes{ 0, 1, 1,
			1, 2, 2, 1, 3, 3, 1, 4, 4, 1, 5, 5 };
		mesh = std::make_unique<MeshModel>(indexes, positions);

		auto linedRenderer = std::make_unique<LinedRenderer>(vulkanContext);

		linedRenderer->transformUniform.model = ComposeMatrix();
		linedRenderer->UpdateTransformUniformBuffer();

		linedRenderer->propertiesUniform.baseColor = Vector4f(0., 0.5, 0.5, 1.);
		linedRenderer->UpdatePropertiesUniformBuffer();

		renderer = std::move(linedRenderer);

		interactive = false;
		UpdateVertexBuffer();
	}

	virtual Matrix4 ComposeMatrix() const override
	{
		return Matrix4::Translate(position) * Matrix4::Rotate(Vector3f(0., 1., 0.), direction) * Matrix4::Scale(scale);
	}

private:
	Vector3f direction;
};
