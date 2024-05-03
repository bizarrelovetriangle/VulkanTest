#pragma once
#pragma once
#include "../../Renderers/Interfaces/Renderer.h"
#include "../../CAD/MeshModel.h"
#include "../../RenderVisitor.h"
#include "../../Camera.h"

class Object
{
public:
	Object()
	{
	}

	Object(std::unique_ptr<Renderer> renderer)
		: renderer(std::move(renderer))
	{
	}

	virtual ~Object() = default;

	virtual void Render(RenderVisitor& renderVisitor, const Camera& camera)
	{
		renderer->transformUniform.model = ComposeMatrix();
		renderer->Accept(renderVisitor, camera);
	}

	virtual void Dispose()
	{
		if (renderer) renderer->Dispose();
	}

	virtual Matrix4 ComposeMatrix() const
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

	bool interactive = true;

	std::unique_ptr<Renderer> renderer;
};
