#pragma once
#include "../Objects/Interfaces/MeshObject.h"
#include "../Renderers/ColoredRenderer.h"
#include "../Renderers/LinedRenderer.h"
#include "../Renderers/SimpleVertexedRenderer.h"
#include <unordered_set>
#include <optional>
#include <queue>
#include "GeometryFunctions.h"

struct ContactInfo
{
	bool contact = false;
	std::shared_ptr<MeshModel> gjkTriangular;
	std::pair<std::shared_ptr<MeshObject>, std::shared_ptr<MeshObject>> objects;
};

class MeshContactAlgorithms
{
public:
	MeshContactAlgorithms(VulkanContext& vulkanContext)
		: vulkanContext(vulkanContext)
	{
	}

	ContactInfo GJK(const MeshObject& objectA, const MeshObject& objectB)
	{
		Dispose();

		//objectA - icosphere, objectB - sphere

		// todo: get rid of it. I need reversed matrix for this
		auto modelA = objectA.ComposeMatrix();
		auto meshA = MeshModel(*objectA.mesh);
		for (auto& point : meshA.points) {
			auto vec4 = modelA * Vector4f(point, 1.);
			point = Vector3f(vec4.x, vec4.y, vec4.z);
		}

		auto modelB = objectB.ComposeMatrix();
		auto meshB = MeshModel(*objectB.mesh);
		for (auto& point : meshB.points) {
			auto vec4 = modelB * Vector4f(point, 1.);
			point = Vector3f(vec4.x, vec4.y, vec4.z);
		}

		ContactInfo result;

		MeshModel minkowskiMesh;

		// maybe from a to b?
		auto direction = Vector3f(1., 0., 0.);
		auto minkowskiDiffA = MinkowskiDiff(direction, meshA, meshB);
		minkowskiMesh.points.push_back(minkowskiDiffA);

		// maybe to the zero?
		direction = -direction;
		auto minkowskiDiffB = MinkowskiDiff(direction, meshA, meshB);
		minkowskiMesh.points.push_back(minkowskiDiffB);

		direction = Vector3f(0., 0., 1.);
		auto minkowskiDiffC = MinkowskiDiff(direction, meshA, meshB);
		minkowskiMesh.points.push_back(minkowskiDiffC);

		uint32_t tri = minkowskiMesh.AddTriangle({ 0, 1, 2 });
		auto triPoints = minkowskiMesh.TrianglePoints(tri);

		// face the triangle to the zero point
		if (GeometryFunctions::TrianglePlanePointDist(triPoints[0], triPoints[1], triPoints[2], Vector3f::Zero()) < 0.)
			std::swap(minkowskiMesh.triangles[tri].vertices[0], minkowskiMesh.triangles[tri].vertices[2]);

		uint32_t nearestTri = tri;

		for (int i = 0; i < 10; ++i)
		{
			auto nearestTriPoints = minkowskiMesh.TrianglePoints(nearestTri);
			
			direction = GeometryFunctions::TrianglePointDir(nearestTriPoints[0], nearestTriPoints[1], nearestTriPoints[2], Vector3f::Zero());
			auto minkowskiDiff = MinkowskiDiff(direction, meshA, meshB);

			if (minkowskiDiff == nearestTriPoints[0] || minkowskiDiff == nearestTriPoints[1] || minkowskiDiff == nearestTriPoints[2])
				break;

			uint32_t newPoint = minkowskiMesh.points.size();
			minkowskiMesh.points.push_back(minkowskiDiff);
			auto nearestTriVerts = minkowskiMesh.triangles[nearestTri].vertices;

			//for (uint32_t tri = 0; tri < minkowskiMesh.triangleBitVector.size(); ++tri)
			//	minkowskiMesh.triangleBitVector[tri] = false;

			uint32_t triA = minkowskiMesh.AddTriangle({ nearestTriVerts[0], nearestTriVerts[1], newPoint });
			uint32_t triB = minkowskiMesh.AddTriangle({ nearestTriVerts[1], nearestTriVerts[2], newPoint });
			uint32_t triC = minkowskiMesh.AddTriangle({ nearestTriVerts[2], nearestTriVerts[0], newPoint });

			// check only the newly created triangular pyramid
			if (GeometryFunctions::PointInsideTriangularPyramid(Vector3f::Zero(), minkowskiMesh, { triA, triB, triC }))
			{
				result.contact = true;
				break;
			}

			auto triangles = { triA, triB, triC };
			auto newTriangle = std::find_if(triangles.begin(), triangles.end(), [&](uint32_t tri)
				{
					auto triPoints = minkowskiMesh.TrianglePoints(tri);
					return GeometryFunctions::TrianglePlanePointDist(triPoints[0], triPoints[1], triPoints[2], Vector3f::Zero()) > 0.;
				});

			if (newTriangle == triangles.end()) break;
			nearestTri = *newTriangle;
		}

		result.gjkTriangular = std::make_shared<MeshModel>(minkowskiMesh);
		std::swap(minkowskiMesh.triangles[0].vertices[0], minkowskiMesh.triangles[0].vertices[2]);
		CreateObject(minkowskiMesh);
		return result;
	}

	Vector3f MinkowskiDiff(const Vector3f& direction, const MeshModel& meshA, const MeshModel& meshB)
	{
		size_t antagonistA = FarthestPoint(direction, meshA.points);
		size_t antagonistB = FarthestPoint(-direction, meshB.points);
		return meshA.points[antagonistA] - meshB.points[antagonistB];
	}

	size_t FarthestPoint(const Vector3f& direction, const std::vector<Vector3f>& points)
	{
		auto farthest = std::make_pair(-(std::numeric_limits<float>::max)(), size_t(0));
		for (size_t i = 0; i < points.size(); ++i) {
			auto pair = std::make_pair(direction.Dot(points[i]), i);
			farthest = (std::max)(farthest, pair);
		}
		return farthest.second;
	}

	void CreateObject(const MeshModel& mesh)
	{
		auto renderer = std::make_unique<SimpleVertexedRenderer>(vulkanContext);
		auto minkowskiObj = std::make_unique<MeshObject>(std::make_unique<MeshModel>(mesh), std::move(renderer));

		minkowskiObj->renderer->propertiesUniform.baseColor = Vector4(0.3, 0.1, 0.1, 1.);
		minkowskiObj->renderer->UpdatePropertiesUniformBuffer();
		minkowskiObj->UpdateVertexBuffer();
		Renderers.insert(std::move(minkowskiObj));
	}

	void Render(RenderVisitor& renderVisitor, const Camera& camera)
	{
		for (auto& Renderer : Renderers)
			Renderer->Render(renderVisitor, camera);
	}

	void Dispose()
	{
		for (auto& Renderer : Renderers)
			Renderer->Dispose();

		Renderers.clear();
	}

private:
	VulkanContext& vulkanContext;
	std::unordered_set<std::shared_ptr<MeshObject>> Renderers;
};
