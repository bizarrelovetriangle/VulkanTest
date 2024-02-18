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
		matrix = Matrix4::Rotate(Vector4f::QuaternionFromGLTF(rotation)) * matrix;
		matrix = Matrix4::Translation(Vector3f::FromGLTF(position)) * matrix;
		return matrix;
	}

	std::string name;

	Vector3f position;
	Vector4f rotation;
	Vector3f velocity;
	Vector4f angularVelocity;

	Vector3f scale;

	std::unique_ptr<RenderObject> renderer;
};
