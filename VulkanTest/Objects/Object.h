#pragma once
#include "../RenderObjects/Interfaces/VertexedRenderObject.h"
#include "../CAD/MeshModel.h"

class Object
{
public:
	Object(std::unique_ptr<MeshModel> mesh, std::unique_ptr<VertexedRenderObject> renderer)
		: mesh(std::move(mesh)), renderer(std::move(renderer))
	{

	}

	void Dispose()
	{
		renderer->Dispose();
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

	std::unique_ptr<MeshModel> mesh;
	std::unique_ptr<VertexedRenderObject> renderer;
};
