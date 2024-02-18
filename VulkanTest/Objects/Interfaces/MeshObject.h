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

	void UpdateVertexBuffer()
	{
		auto& vertexedRendered = *((VertexedRenderObject*)renderer.get());
		vertexedRendered.UpdateVertexBuffer(*mesh);
	}

	std::unique_ptr<MeshModel> mesh;
};
