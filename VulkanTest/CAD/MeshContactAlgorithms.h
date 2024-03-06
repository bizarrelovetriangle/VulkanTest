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

		if (TranglePlanePointDist(minkowskiMesh.points[0], minkowskiMesh.points[1], minkowskiMesh.points[2], Vector3f(0., 0., 0.)) > 0.)
		{
			std::swap(minkowskiMesh.points[0], minkowskiMesh.points[2]);
		}

		minkowskiMesh.AddTriangle({ 0, 1, 2 });

		{
			direction = -TranglePointDir(minkowskiMesh.points[0], minkowskiMesh.points[1], minkowskiMesh.points[2], Vector3f(0., 0., 0.));
			auto minkowskiDiff = MinkowskiDiff(direction, meshA, meshB);
			minkowskiMesh.points.push_back(minkowskiDiff);

			minkowskiMesh.AddTriangle({ 0, 3, 1 });
			minkowskiMesh.AddTriangle({ 1, 3, 2 });
			minkowskiMesh.AddTriangle({ 2, 3, 0 });

			using type = std::pair<float, int64_t>;
			auto less = [](type& a, type& b) { return a.first < b.first; };
			std::priority_queue<type, std::vector<type>, decltype(less)> triangleDistPQ(less);

			// check only the newly created triangular pyramid
			bool checkSuccess = PointInsideTriangularPyramid(Vector3f(0., 0., 0.), minkowskiMesh, {0, 1, 2, 3});
			if (checkSuccess)
			{
				CreateObject(minkowskiMesh);
				return ContactInfo{ .contact = true };
			}

			// remove unnecessary triangles from the mesh
		}

		CreateObject(minkowskiMesh);

		return {};
	}

	Vector3f MinkowskiDiff(const Vector3f& direction, const MeshModel& meshA, const MeshModel& meshB)
	{
		size_t antagonistA = FarthestPoint(direction, meshA.points);
		size_t antagonistB = FarthestPoint(-direction, meshB.points);
		return meshB.points[antagonistB] - meshA.points[antagonistA];
	}

	size_t FarthestPoint(const Vector3f& direction, const std::vector<Vector3f>& points)
	{
		auto farthest = std::make_pair((std::numeric_limits<float>::min)(), size_t(0));
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
