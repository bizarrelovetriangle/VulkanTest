#pragma once
#include "../Objects/Interfaces/MeshObject.h"
#include "../RenderObjects/ColoredRenderObject.h"
#include "../RenderObjects/LinedRenderObject.h"
#include <unordered_set>
#include <optional>
#include <queue>

struct ContactInfo
{
	bool contact = false;
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

		using type = std::pair<float, uint32_t>;
		auto less = [](type& a, type& b) { return a.first > b.first; };
		std::priority_queue<type, std::vector<type>, decltype(less)> triangleDistPQ(less);

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

		// the first triangle will face the opposide direction of zero point
		if (TranglePlanePointDist(triPoints[0], triPoints[1], triPoints[2], Vector3f::Zero()) > 0.) {
			std::swap(minkowskiMesh.triangles[tri].vertices[0], minkowskiMesh.triangles[tri].vertices[2]);
		}

		float dist = TranglePointDist(triPoints[0], triPoints[1], triPoints[2], Vector3f::Zero());
		triangleDistPQ.emplace(dist, tri);

		float nearest = (std::numeric_limits<float>::max)();

		for (int i = 0; i < 2; ++i)
		{
			auto [dist, nearestTri] = triangleDistPQ.top();
			if (i != 0) triangleDistPQ.pop();
			if (dist >= nearest)
				break;
			nearest = (std::min)(nearest, dist);

			auto triPoints = minkowskiMesh.TrianglePoints(nearestTri);
			direction = TranglePointDir(triPoints[0], triPoints[1], triPoints[2], Vector3f::Zero());
			auto minkowskiDiff = MinkowskiDiff(direction, meshA, meshB);

			uint32_t newPoint = minkowskiMesh.points.size();
			minkowskiMesh.points.push_back(minkowskiDiff);

			auto nearestTriVerts = minkowskiMesh.triangles[nearestTri].vertices;
			// we consider the nearest triangle as it was faced toward the zero point. this is not the case for the first triangle
			if (i == 0)
				std::swap(nearestTriVerts[0], nearestTriVerts[2]);

			uint32_t triA = minkowskiMesh.AddTriangle({ nearestTriVerts[0], nearestTriVerts[1], newPoint });
			uint32_t triB = minkowskiMesh.AddTriangle({ nearestTriVerts[1], nearestTriVerts[2], newPoint });
			uint32_t triC = minkowskiMesh.AddTriangle({ nearestTriVerts[2], nearestTriVerts[0], newPoint });

			for (uint32_t tri : { triA, triB, triC })
			{
				auto triPoints = minkowskiMesh.TrianglePoints(tri);
				float dist = TranglePointDist(triPoints[0], triPoints[1], triPoints[2], Vector3f::Zero());
				triangleDistPQ.emplace(dist, tri);
			}

			// check only the newly created triangular pyramid
			bool checkSuccess = PointInsideTriangularPyramid(Vector3f::Zero(), minkowskiMesh, { nearestTri, triA, triB, triC });
			if (checkSuccess)
			{
				CreateObject(minkowskiMesh);
				return ContactInfo{ .contact = true };
			}

			//remove inner triangle
			//if (i != 0) minkowskiMesh.DeleteTriangle(nearestTri);
		}

		CreateObject(minkowskiMesh);

		return {};
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
			float dist = TranglePlanePointDist(
				meshPoints[triVerts[0]], meshPoints[triVerts[1]], meshPoints[triVerts[2]], point);
			if (dist > 0) return false;
		}
		return true;
	}

	float TranglePlanePointDist(const Vector3f& a, const Vector3f& b, const Vector3f& c, const Vector3f& point)
	{
		auto normal = (b - a).Cross(c - a).Normalized();
		return normal.Dot(point - a);
	}

	float TranglePointDist(const Vector3f& a, const Vector3f& b, const Vector3f& c, const Vector3f& point)
	{
		auto normal = (b - a).Cross(c - a).Normalized();
		return std::abs(normal.Dot(point - a));
	}

	float TranglePointDist2(const Vector3f& a, const Vector3f& b, const Vector3f& c, const Vector3f& point)
	{
		float minPointDist = std::sqrt((std::min)({ (a - point).Length2(), (b - point).Length2(), (c - point).Length2() }));
		auto normal = (b - a).Cross(c - a).Normalized();
		float planeDist = std::abs(normal.Dot(point - a));
		return (std::max)(minPointDist, planeDist);
	}

	Vector3f TranglePointDir(const Vector3f& a, const Vector3f& b, const Vector3f& c, const Vector3f& point)
	{
		auto normal = (b - a).Cross(c - a).Normalized();
		float planeDist = normal.Dot(point - a);
		return normal * planeDist;
	}

	void CreateObject(const MeshModel& mesh)
	{
		std::vector<Vector4f> colors{
			Vector4f(1., 0., 0., 1.), //R
			Vector4f(0., 1., 0., 1.), //G
			Vector4f(0., 0., 1., 1.), //B
			Vector4f(1., 1., 1., 1.), //W
			Vector4f(1., 1., 0., 1.), //Y
			Vector4f(0., 1., 1., 1.) };//T

		auto renderer = std::make_unique<ColoredRenderObject>(vulkanContext, colors);

		auto minkowskiObj = std::make_unique<MeshObject>(std::make_unique<MeshModel>(mesh), std::move(renderer));
		minkowskiObj->UpdateVertexBuffer();
		minkowskiObj->renderer->propertiesUniform.baseColor = Vector4(0.3, 0.1, 0.1, 1.);
		minkowskiObj->renderer->UpdatePropertiesUniformBuffer();
		renderObjects.insert(std::move(minkowskiObj));
	}

	void Render(RenderVisitor& renderVisitor)
	{
		for (auto& renderObject : renderObjects)
			renderObject->Render(renderVisitor);
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
