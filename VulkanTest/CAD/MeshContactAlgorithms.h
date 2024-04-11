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
	bool contact;
	std::shared_ptr<MeshModel> gjkTriangular;
	std::pair<std::shared_ptr<MeshObject>, std::shared_ptr<MeshObject>> objectPair;
	Vector3f contactPoint;
	Vector3f normal;
};

class MeshContactAlgorithms
{
public:
	MeshContactAlgorithms(VulkanContext& vulkanContext)
		: vulkanContext(vulkanContext)
	{
	}

	ContactInfo CheckContact(std::shared_ptr<MeshObject> objectA, std::shared_ptr<MeshObject> objectB)
	{
		Dispose();

		ContactInfo contactInfo;
		contactInfo.contact = false;
		contactInfo.gjkTriangular = std::make_shared<MeshModel>();
		contactInfo.objectPair = std::make_pair(objectA, objectB);

		//objectA - icosphere, objectB - sphere
		// todo: get rid of it. I need reversed matrix for this
		auto modelA = objectA->ComposeMatrix();
		auto meshA = MeshModel(*objectA->mesh);
		for (auto& point : meshA.points)
			point = modelA * Vector4f(point, 1.);

		auto modelB = objectB->ComposeMatrix();
		auto meshB = MeshModel(*objectB->mesh);
		for (auto& point : meshB.points)
			point = modelB * Vector4f(point, 1.);

		GJK(meshA, meshB, contactInfo);

		if (contactInfo.contact)
		{
			EPA(meshA, meshB, contactInfo);
			Clipping(meshA, meshB, contactInfo);
		}

		return contactInfo;
	}

	void GJK(const MeshModel& meshA, const MeshModel& meshB, ContactInfo& contactInfo)
	{
		// maybe from a to b?
		auto direction = Vector3f(1., 0., 0.);
		auto minkowskiDiffA = MinkowskiDiff(direction, meshA, meshB);
		contactInfo.gjkTriangular->points.push_back(minkowskiDiffA);

		// maybe to the zero?
		direction = -direction;
		auto minkowskiDiffB = MinkowskiDiff(direction, meshA, meshB);
		contactInfo.gjkTriangular->points.push_back(minkowskiDiffB);

		direction = Vector3f(0., 0., 1.);
		auto minkowskiDiffC = MinkowskiDiff(direction, meshA, meshB);
		contactInfo.gjkTriangular->points.push_back(minkowskiDiffC);

		uint32_t nearestTri = contactInfo.gjkTriangular->AddTriangle({ 0, 1, 2 });

		// face the triangle to the zero point
		auto triPoints = contactInfo.gjkTriangular->TrianglePoints(nearestTri);
		if (GeometryFunctions::TrianglePlanePointDist(triPoints[0], triPoints[1], triPoints[2], Vector3f::Zero()) < 0.)
			std::swap(contactInfo.gjkTriangular->triangles[nearestTri].vertices[0], contactInfo.gjkTriangular->triangles[nearestTri].vertices[2]);

		for (int i = 0; i < 10; ++i)
		{
			auto nearestTriPoints = contactInfo.gjkTriangular->TrianglePoints(nearestTri);

			direction = GeometryFunctions::TrianglePointDir(nearestTriPoints[0], nearestTriPoints[1], nearestTriPoints[2], Vector3f::Zero());
			auto minkowskiDiff = MinkowskiDiff(direction, meshA, meshB);

			if (minkowskiDiff == nearestTriPoints[0] || minkowskiDiff == nearestTriPoints[1] || minkowskiDiff == nearestTriPoints[2])
				break;

			uint32_t newPoint = contactInfo.gjkTriangular->points.size();
			contactInfo.gjkTriangular->points.push_back(minkowskiDiff);
			auto nearestTriVerts = contactInfo.gjkTriangular->triangles[nearestTri].vertices;

			for (uint32_t tri = 0; tri < contactInfo.gjkTriangular->triangleBitVector.size(); ++tri)
				if (tri != nearestTri)
					contactInfo.gjkTriangular->triangleBitVector[tri] = false;

			uint32_t triA = contactInfo.gjkTriangular->AddTriangle({ nearestTriVerts[0], nearestTriVerts[1], newPoint });
			uint32_t triB = contactInfo.gjkTriangular->AddTriangle({ nearestTriVerts[1], nearestTriVerts[2], newPoint });
			uint32_t triC = contactInfo.gjkTriangular->AddTriangle({ nearestTriVerts[2], nearestTriVerts[0], newPoint });

			// check only the newly created triangular pyramid
			if (GeometryFunctions::PointInsideTriangularPyramid(Vector3f::Zero(), *contactInfo.gjkTriangular, { triA, triB, triC }))
			{
				contactInfo.contact = true;
				std::swap(contactInfo.gjkTriangular->triangles[nearestTri].vertices[0], contactInfo.gjkTriangular->triangles[nearestTri].vertices[2]);
				break;
			}

			auto triangles = { triA, triB, triC };
			auto newTriangle = std::find_if(triangles.begin(), triangles.end(), [&](uint32_t tri)
				{
					auto triPoints = contactInfo.gjkTriangular->TrianglePoints(tri);
					return GeometryFunctions::TrianglePlanePointDist(triPoints[0], triPoints[1], triPoints[2], Vector3f::Zero()) > 0.;
				});

			if (newTriangle == triangles.end()) break;
			nearestTri = *newTriangle;
		}

		//CreateObject(*contactInfo.gjkTriangular);
	}

	void EPA(const MeshModel& meshA, const MeshModel& meshB, ContactInfo& contactInfo)
	{
		using type = std::pair<float, uint32_t>;
		auto less = [](type& a, type& b) { return a.first > b.first; };
		std::priority_queue<type, std::vector<type>, decltype(less)> triangleDistPQ(less);

		for (auto& tri : contactInfo.gjkTriangular->triangles) {
			if (!contactInfo.gjkTriangular->triangleBitVector[tri.index]) continue;
			auto triPoints = contactInfo.gjkTriangular->TrianglePoints(tri.index);
			float dist = GeometryFunctions::TrianglePlanePointDist(triPoints[0], triPoints[1], triPoints[2], Vector3f::Zero());
			triangleDistPQ.emplace(-dist, tri.index);
		}

		type bestSoFar = std::make_pair(-(std::numeric_limits<float>::max)(), 0);
		while (true)
		{
			bestSoFar = triangleDistPQ.top();
			auto& [dist, nearestTri] = bestSoFar;
			triangleDistPQ.pop();

			contactInfo.gjkTriangular->triangleBitVector[nearestTri] = false;

			auto nearestTriPoints = contactInfo.gjkTriangular->TrianglePoints(nearestTri);
			auto direction = contactInfo.gjkTriangular->TriangleNormal(nearestTri);
			auto minkowskiDiff = MinkowskiDiff(direction, meshA, meshB);

			if (minkowskiDiff == nearestTriPoints[0] || minkowskiDiff == nearestTriPoints[1] || minkowskiDiff == nearestTriPoints[2])
				break;

			uint32_t newPoint = contactInfo.gjkTriangular->points.size();
			contactInfo.gjkTriangular->points.push_back(minkowskiDiff);

			auto nearestTriVerts = contactInfo.gjkTriangular->triangles[nearestTri].vertices;
			uint32_t triA = contactInfo.gjkTriangular->AddTriangle({ nearestTriVerts[0], nearestTriVerts[1], newPoint });
			uint32_t triB = contactInfo.gjkTriangular->AddTriangle({ nearestTriVerts[1], nearestTriVerts[2], newPoint });
			uint32_t triC = contactInfo.gjkTriangular->AddTriangle({ nearestTriVerts[2], nearestTriVerts[0], newPoint });

			for (uint32_t tri : { triA, triB, triC })
			{
				auto triPoints = contactInfo.gjkTriangular->TrianglePoints(tri);
				float dist = GeometryFunctions::TrianglePlanePointDist(triPoints[0], triPoints[1], triPoints[2], Vector3f::Zero());
				if (dist > 0.) {
					contactInfo.gjkTriangular->triangleBitVector[tri] = false;
					continue;
				}

				triangleDistPQ.emplace(-dist, tri);
			}
		}

		contactInfo.normal = contactInfo.gjkTriangular->TriangleNormal(bestSoFar.second);

		auto mesh = *contactInfo.gjkTriangular;
		for (uint32_t tri = 0; tri < contactInfo.gjkTriangular->triangleBitVector.size(); ++tri)
			mesh.triangleBitVector[tri] = false;
		mesh.triangleBitVector[bestSoFar.second] = true;

		CreateObject(std::move(mesh));
	}

	void Clipping(const MeshModel& meshA, const MeshModel& meshB, ContactInfo& contactInfo)
	{

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
		auto renderer = std::make_unique<LinedRenderer>(vulkanContext);
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
