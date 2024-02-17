#pragma once
#include "../../RenderObjects/Interfaces/VertexedRenderObject.h"
#include "../../CAD/MeshModel.h"
#include "Object.h"

class MeshObject : public Object
{
public:
	MeshObject()
	{
	}

	MeshObject(std::unique_ptr<MeshModel> mesh, std::unique_ptr<VertexedRenderObject> renderer)
		: mesh(std::move(mesh)), Object(std::move(renderer))
	{

	}

	Matrix4 ComposeMatrix() const
	{
		Matrix4 matrix;
		matrix = Matrix4::Scale(scale) * matrix;
		matrix = Matrix4::Rotate(Vector4f::QuaternionFromGLTF(rotation)) * matrix;
		matrix = Matrix4::Translation(Vector3f::FromGLTF(position)) * matrix;
		return matrix;
	}

	void UpdateVertexBuffer()
	{
		auto& vertexedRendered = *((VertexedRenderObject*)renderer.get());
		vertexedRendered.UpdateVertexBuffer(*mesh);
	}

	std::unique_ptr<MeshModel> mesh;
};
