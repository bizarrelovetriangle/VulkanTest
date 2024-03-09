#pragma once
#pragma once
#include "../../RenderObjects/Interfaces/RenderObject.h"
#include "../../CAD/MeshModel.h"
#include "../../RenderVisitor.h"

class Object
{
public:
	Object()
	{
	}

	Object(std::unique_ptr<RenderObject> renderer)
		: renderer(std::move(renderer))
	{
	}

	virtual ~Object() = default;

	virtual void Render(RenderVisitor& renderVisitor)
	{
		renderer->Accept(renderVisitor);
	}

	virtual void Dispose()
	{
		if (renderer) renderer->Dispose();
	}

	Matrix4 ComposeMatrix() const
	{
		Matrix4 matrix;
		matrix = Matrix4::Scale(scale) * matrix;
		matrix = Matrix4::Rotate(rotation) * matrix;
		matrix = Matrix4::Translation(position) * matrix;
		return matrix;
	}

	std::string name;

	Vector3f position;
	Vector4f rotation = Vector4f(0., 0., 0., 1.);
	Vector3f velocity;
	Vector4f angularVelocity;

	Vector3f scale = Vector3f(1., 1., 1.);

	std::unique_ptr<RenderObject> renderer;
};
