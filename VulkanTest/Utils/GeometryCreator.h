#pragma once
#include <vector>
#include "../RenderObjects/Interfaces/VertexedRenderObject.h"
#include "../RenderObjects/Primitives/BoundingBoxObject.h"

class GeometryCreator
{
public:
	static std::vector<VertexData> createBoxByTwoPoints(const Vector3f& aa, const Vector3f& bb);
	static std::vector<LineVertexData> createLinedBoxByTwoPoints(const Vector3f& aa, const Vector3f& bb);
};