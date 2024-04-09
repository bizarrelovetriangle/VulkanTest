#pragma once
#include <vector>
#include "../RenderObjects/Interfaces/VertexedRenderObject.h"

class MeshModel;

class GeometryCreator
{
public:
	static std::unique_ptr<MeshModel> CreateIcosphere(float radius = 1., int subdivisions = 1);
	static std::unique_ptr<MeshModel> CreateBox(const Vector3f& size);
	static std::unique_ptr<MeshModel> CreateBoxByTwoPoints(const Vector3f& aa, const Vector3f& bb);
	static std::vector<Vector3f> CreateLinedBoxByTwoPoints(const Vector3f& aa, const Vector3f& bb);
};