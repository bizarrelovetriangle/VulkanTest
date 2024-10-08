#pragma once
#include <vector>
#include "../Renderers/Interfaces/VertexedRenderer.h"

class MeshModel;

class GeometryCreator
{
public:
	static std::unique_ptr<MeshModel> CreateIcosphere(float radius = 1., int subdivisions = 1);
	static std::unique_ptr<MeshModel> CreateBox(const Vector3f& size);
	static std::unique_ptr<MeshModel> CreateBoxByTwoPoints(const Vector3f& aa, const Vector3f& bb);
	static std::unique_ptr<MeshModel> CreateLinedBoxByTwoPoints(const Vector3f& aa, const Vector3f& bb);
};