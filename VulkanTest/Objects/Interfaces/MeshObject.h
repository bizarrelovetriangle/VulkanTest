#pragma once
#include "../../Renderers/Interfaces/VertexedRenderer.h"
#include "../../CAD/MeshModel.h"
#include "Object.h"

class MeshObject : public Object
{
public:
	MeshObject()
	{
	}

	MeshObject(std::unique_ptr<MeshModel> mesh, std::unique_ptr<VertexedRenderer> renderer)
		: mesh(std::move(mesh)), Object(std::move(renderer))
	{
		UpdateVertexBuffer();
	}

	void UpdateVertexBuffer()
	{
		auto& vertexedRendered = *((VertexedRenderer*)renderer.get());
		vertexedRendered.UpdateVertexBuffer(*mesh);
	}

	std::shared_ptr<MeshModel> mesh;
	std::vector<std::shared_ptr<MeshModel>> convexSegments;
};
