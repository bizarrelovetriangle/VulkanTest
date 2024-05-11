#pragma once
#include "../../Renderers/Interfaces/VertexedRenderer.h"
#include "../../CAD/MeshModel.h"
#include "Object.h"
#include "../../CAD/GeometryFunctions.h"

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

	bool Intersect(const Vector3f& segmentA, const Vector3f& segmentB, std::pair<float, Vector3f>* nearestPos) const
	{
		auto modelToWorld = ComposeMatrix();
		auto worldToModel = modelToWorld.Inverse();

		Vector3f modelSpaceSegmentA = worldToModel * Vector4f(segmentA, 1.);
		Vector3f modelSpaceSegmentB = worldToModel * Vector4f(segmentB, 1.);

		if (nearestPos) {
			*nearestPos = { (std::numeric_limits<float>::max)(), {} };
		}

		bool success = false;

		for (int i = 0; i < mesh->triangles.size(); ++i)
		{
			if (!mesh->triangleBitVector[i]) continue;
			auto points = mesh->TrianglePoints(i);

			float dist = 0.;
			Vector3f intersectPoint;

			if (GeometryFunctions::SegmentTriangleIntersetion(
				modelSpaceSegmentA, modelSpaceSegmentB, points[0], points[1], points[2], intersectPoint, &dist))
			{
				if (!nearestPos) {
					return true;
				}

				success = true;
				if (dist < nearestPos->first)
				{
					auto pos = modelToWorld * Vector4f(intersectPoint, 1.);
					*nearestPos = { dist, pos };
				}
			}
		}

		return success;
	}

	std::unique_ptr<MeshModel> mesh;
	std::vector<MeshModel> convexSegments;
};
