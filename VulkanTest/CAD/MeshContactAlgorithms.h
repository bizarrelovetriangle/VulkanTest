#pragma once
#include "../Objects/Interfaces/MeshObject.h"
#include "../RenderObjects/ColoredRenderObject.h"
#include "../RenderObjects/LinedRenderObject.h"
#include "../RenderObjects/SimpleVertexedRenderObject.h"
#include <unordered_set>
#include <optional>
#include <queue>

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
		if (TrianglePlanePointDist(triPoints[0], triPoints[1], triPoints[2], Vector3f::Zero()) < 0.)
			std::swap(minkowskiMesh.triangles[tri].vertices[0], minkowskiMesh.triangles[tri].vertices[2]);

		uint32_t nearestTri = tri;

		for (int i = 0; i < 10; ++i)
		{
			auto nearestTriPoints = minkowskiMesh.TrianglePoints(nearestTri);
			
			direction = TrianglePointDir(nearestTriPoints[0], nearestTriPoints[1], nearestTriPoints[2], Vector3f::Zero());
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
			if (PointInsideTriangularPyramid(Vector3f::Zero(), minkowskiMesh, { triA, triB, triC }))
			{
				result.contact = true;
				break;
			}

			auto triangles = { triA, triB, triC };
			auto newTriangle = std::find_if(triangles.begin(), triangles.end(), [&](uint32_t tri)
				{
					auto triPoints = minkowskiMesh.TrianglePoints(tri);
					return TrianglePlanePointDist(triPoints[0], triPoints[1], triPoints[2], Vector3f::Zero()) > 0.;
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

	bool PointInsideTriangularPyramid(const Vector3f& point, const MeshModel& mesh, const std::vector<uint32_t> triIndexes)
	{
		auto& meshPoints = mesh.points;
		for (uint32_t triIndex : triIndexes)
		{
			auto& triVerts = mesh.triangles[triIndex].vertices;
			float dist = TrianglePlanePointDist(
				meshPoints[triVerts[0]], meshPoints[triVerts[1]], meshPoints[triVerts[2]], point);
			if (dist > 0) return false;
		}
		return true;
	}

	float TrianglePlanePointDist(const Vector3f& a, const Vector3f& b, const Vector3f& c, const Vector3f& point)
	{
		auto normal = (b - a).Cross(c - a).Normalized();
		return normal.Dot(point - a);
	}

	float TrianglePointDist(const Vector3f& a, const Vector3f& b, const Vector3f& c, const Vector3f& point)
	{
		// here we check if the point is inside a prism that created with sides and normal of the triangle
		// if the point is outside two sides simultaniously, calculating the distance for only one of them will be sufficient
		auto triNormal = (b - a).Cross(c - a);
		Vector3f a_b_point_vector = (b - a).Cross(point - a);
		Vector3f b_c_point_vector = (c - b).Cross(point - b);
		Vector3f c_a_point_vector = (a - c).Cross(point - c);

		auto test1 = triNormal.Dot(a_b_point_vector) > 0;
		auto test2 = triNormal.Dot(b_c_point_vector) > 0;
		auto test3 = triNormal.Dot(c_a_point_vector) > 0;

		if (!test1) return LinePointDist(a, b, point);
		if (!test2) return LinePointDist(b, c, point);
		if (!test3) return LinePointDist(c, a, point);

		auto normal = (b - a).Cross(c - a).Normalized();
		float planeDist = std::abs(normal.Dot(point - a));
		return planeDist;
	}

	float LinePointDist(const Vector3f& a, const Vector3f& b, const Vector3f& point)
	{
		auto a_b = b - a;
		auto a_b_normalized = a_b.Normalized();
		auto proj_length = a_b_normalized.Dot(point - a);
		if (proj_length < 0.) return (point - a).Length();
		if (proj_length > a_b.Length()) return (point - b).Length();
		return (a_b_normalized * proj_length + a - point).Length();
	}

	Vector3f TrianglePointDir(const Vector3f& a, const Vector3f& b, const Vector3f& c, const Vector3f& point)
	{
		auto normal = (b - a).Cross(c - a).Normalized();
		float planeDist = normal.Dot(point - a);
		return (normal * planeDist).Normalized();
	}

	void CreateObject(const MeshModel& mesh)
	{
		auto renderer = std::make_unique<SimpleVertexedRenderObject>(vulkanContext);
		auto minkowskiObj = std::make_unique<MeshObject>(std::make_unique<MeshModel>(mesh), std::move(renderer));

		minkowskiObj->renderer->propertiesUniform.baseColor = Vector4(0.3, 0.1, 0.1, 1.);
		minkowskiObj->renderer->UpdatePropertiesUniformBuffer();
		minkowskiObj->UpdateVertexBuffer();
		renderObjects.insert(std::move(minkowskiObj));
	}

	void Render(RenderVisitor& renderVisitor, const Camera& camera)
	{
		for (auto& renderObject : renderObjects)
			renderObject->Render(renderVisitor, camera);
	}

	void Dispose()
	{
		for (auto& renderObject : renderObjects)
			renderObject->Dispose();

		renderObjects.clear();
	}

private:
	VulkanContext& vulkanContext;
	std::unordered_set<std::shared_ptr<MeshObject>> renderObjects;
};
