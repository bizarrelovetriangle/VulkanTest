#pragma once
#include <vector>
#include "../RenderObjects/Interfaces/VertexedRenderObject.h"

class MeshModel;

class GeometryCreator
{
public:
	static std::unique_ptr<MeshModel> createBoxByTwoPoints(const Vector3f& aa, const Vector3f& bb);
	static std::vector<Vector3f> createLinedBoxByTwoPoints(const Vector3f& aa, const Vector3f& bb);
};