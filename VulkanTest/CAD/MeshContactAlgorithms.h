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
	std::shared_ptr<MeshModel> minkowskiTriangular;
	std::vector<std::pair<uint32_t, uint32_t>> minkowskiOrgPointPairs;
	std::pair<std::shared_ptr<MeshObject>, std::shared_ptr<MeshObject>> objectPair;
	uint32_t bestMinkowskiTriangle = 0;
	Vector3f contactPoint;
	Vector3f normal;
};

class MeshContactAlgorithms
{
private:
	struct MinkowskiDiff
	{
		Vector3f diff;
		std::pair<uint32_t, uint32_t> pointPair;
	};

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
		contactInfo.minkowskiTriangular = std::make_shared<MeshModel>();
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
		auto minkowskiDiffA = GetMinkowskiDiff(direction, meshA, meshB);
		contactInfo.minkowskiTriangular->points.push_back(minkowskiDiffA.diff);
		contactInfo.minkowskiOrgPointPairs.push_back(minkowskiDiffA.pointPair);

		// maybe to the zero?
		direction = -direction;
		auto minkowskiDiffB = GetMinkowskiDiff(direction, meshA, meshB);
		contactInfo.minkowskiTriangular->points.push_back(minkowskiDiffB.diff);
		contactInfo.minkowskiOrgPointPairs.push_back(minkowskiDiffB.pointPair);

		direction = Vector3f(0., 0., 1.);
		auto minkowskiDiffC = GetMinkowskiDiff(direction, meshA, meshB);
		contactInfo.minkowskiTriangular->points.push_back(minkowskiDiffC.diff);
		contactInfo.minkowskiOrgPointPairs.push_back(minkowskiDiffC.pointPair);

		uint32_t nearestTri = contactInfo.minkowskiTriangular->AddTriangle({ 0, 1, 2 });

		// face the triangle to the zero point
		auto triPoints = contactInfo.minkowskiTriangular->TrianglePoints(nearestTri);
		if (GeometryFunctions::TrianglePlanePointDist(triPoints[0], triPoints[1], triPoints[2], Vector3f::Zero()) < 0.)
			std::swap(contactInfo.minkowskiTriangular->triangles[nearestTri].vertices[0], contactInfo.minkowskiTriangular->triangles[nearestTri].vertices[2]);

		for (int i = 0; i < 10; ++i)
		{
			auto nearestTriPoints = contactInfo.minkowskiTriangular->TrianglePoints(nearestTri);

			direction = GeometryFunctions::TrianglePointDir(nearestTriPoints[0], nearestTriPoints[1], nearestTriPoints[2], Vector3f::Zero());
			auto minkowskiDiff = GetMinkowskiDiff(direction, meshA, meshB);

			if (minkowskiDiff.diff == nearestTriPoints[0] || minkowskiDiff.diff == nearestTriPoints[1] || minkowskiDiff.diff == nearestTriPoints[2])
				break;

			uint32_t newPoint = contactInfo.minkowskiTriangular->points.size();
			contactInfo.minkowskiTriangular->points.push_back(minkowskiDiff.diff);
			contactInfo.minkowskiOrgPointPairs.push_back(minkowskiDiff.pointPair);

			for (uint32_t tri = 0; tri < contactInfo.minkowskiTriangular->triangleBitVector.size(); ++tri)
				if (tri != nearestTri)
					contactInfo.minkowskiTriangular->triangleBitVector[tri] = false;

			auto nearestTriVerts = contactInfo.minkowskiTriangular->triangles[nearestTri].vertices;
			uint32_t triA = contactInfo.minkowskiTriangular->AddTriangle({ nearestTriVerts[0], nearestTriVerts[1], newPoint });
			uint32_t triB = contactInfo.minkowskiTriangular->AddTriangle({ nearestTriVerts[1], nearestTriVerts[2], newPoint });
			uint32_t triC = contactInfo.minkowskiTriangular->AddTriangle({ nearestTriVerts[2], nearestTriVerts[0], newPoint });

			// check only the newly created triangular pyramid
			if (GeometryFunctions::PointInsideTriangularPyramid(Vector3f::Zero(), *contactInfo.minkowskiTriangular, { triA, triB, triC }))
			{
				contactInfo.contact = true;
				std::swap(contactInfo.minkowskiTriangular->triangles[nearestTri].vertices[0], contactInfo.minkowskiTriangular->triangles[nearestTri].vertices[2]);
				break;
			}

			auto triangles = { triA, triB, triC };
			auto newTriangle = std::find_if(triangles.begin(), triangles.end(), [&](uint32_t tri)
				{
					auto triPoints = contactInfo.minkowskiTriangular->TrianglePoints(tri);
					return GeometryFunctions::TrianglePlanePointDist(triPoints[0], triPoints[1], triPoints[2], Vector3f::Zero()) > 0.;
				});

			if (newTriangle == triangles.end()) break;
			nearestTri = *newTriangle;
		}

		//CreateObject(*contactInfo.minkowskiTriangular);
	}

	void EPA(const MeshModel& meshA, const MeshModel& meshB, ContactInfo& contactInfo)
	{
		using type = std::pair<float, uint32_t>;
		auto less = [](type& a, type& b) { return a.first > b.first; };
		std::priority_queue<type, std::vector<type>, decltype(less)> triangleDistPQ(less);

		for (auto& tri : contactInfo.minkowskiTriangular->triangles) {
			if (!contactInfo.minkowskiTriangular->triangleBitVector[tri.index]) continue;
			auto triPoints = contactInfo.minkowskiTriangular->TrianglePoints(tri.index);
			float dist = GeometryFunctions::TrianglePlanePointDist(triPoints[0], triPoints[1], triPoints[2], Vector3f::Zero());
			triangleDistPQ.emplace(-dist, tri.index);
		}

		type bestSoFar = std::make_pair(-(std::numeric_limits<float>::max)(), 0);
		for (int i = 0; i < 10; ++i)
		{
			bestSoFar = triangleDistPQ.top();
			auto& [dist, nearestTri] = bestSoFar;
			triangleDistPQ.pop();

			contactInfo.minkowskiTriangular->triangleBitVector[nearestTri] = false;

			auto nearestTriPoints = contactInfo.minkowskiTriangular->TrianglePoints(nearestTri);
			auto direction = contactInfo.minkowskiTriangular->TriangleNormal(nearestTri);
			auto minkowskiDiff = GetMinkowskiDiff(direction, meshA, meshB);

			if (minkowskiDiff.diff == nearestTriPoints[0] || minkowskiDiff.diff == nearestTriPoints[1] || minkowskiDiff.diff == nearestTriPoints[2])
				break;

			uint32_t newPoint = contactInfo.minkowskiTriangular->points.size();
			contactInfo.minkowskiTriangular->points.push_back(minkowskiDiff.diff);
			contactInfo.minkowskiOrgPointPairs.push_back(minkowskiDiff.pointPair);

			auto nearestTriVerts = contactInfo.minkowskiTriangular->triangles[nearestTri].vertices;
			uint32_t triA = contactInfo.minkowskiTriangular->AddTriangle({ nearestTriVerts[0], nearestTriVerts[1], newPoint });
			uint32_t triB = contactInfo.minkowskiTriangular->AddTriangle({ nearestTriVerts[1], nearestTriVerts[2], newPoint });
			uint32_t triC = contactInfo.minkowskiTriangular->AddTriangle({ nearestTriVerts[2], nearestTriVerts[0], newPoint });

			for (uint32_t tri : { triA, triB, triC })
			{
				auto triPoints = contactInfo.minkowskiTriangular->TrianglePoints(tri);
				float dist = GeometryFunctions::TrianglePlanePointDist(triPoints[0], triPoints[1], triPoints[2], Vector3f::Zero());
				if (dist > 0.) {
					contactInfo.minkowskiTriangular->triangleBitVector[tri] = false;
					continue;
				}

				triangleDistPQ.emplace(-dist, tri);
			}
		}

		contactInfo.bestMinkowskiTriangle = bestSoFar.second;
		contactInfo.normal = contactInfo.minkowskiTriangular->TriangleNormal(bestSoFar.second);

		auto mesh = MeshModel(*contactInfo.minkowskiTriangular);
		for (uint32_t tri = 0; tri < contactInfo.minkowskiTriangular->triangleBitVector.size(); ++tri)
			mesh.triangleBitVector[tri] = false;
		mesh.triangleBitVector[bestSoFar.second] = true;
		CreateObject(std::move(mesh));
	}

	void Clipping(const MeshModel& meshA, const MeshModel& meshB, ContactInfo& contactInfo)
	{
		auto& orgPoints = contactInfo.minkowskiOrgPointPairs;
		auto& triVerts = contactInfo.minkowskiTriangular->triangles[contactInfo.bestMinkowskiTriangle].vertices;

		uint32_t incidentPoint;

		if (orgPoints[triVerts[0]].first == orgPoints[triVerts[1]].first &&
			orgPoints[triVerts[0]].first == orgPoints[triVerts[2]].first &&
			orgPoints[triVerts[1]].first == orgPoints[triVerts[2]].first)
		{
			incidentPoint = orgPoints[triVerts[0]].first;
		}
		else if(orgPoints[triVerts[0]].second == orgPoints[triVerts[1]].second &&
			orgPoints[triVerts[0]].second == orgPoints[triVerts[2]].second &&
			orgPoints[triVerts[1]].second == orgPoints[triVerts[2]].second)
		{
			incidentPoint = orgPoints[triVerts[0]].second;
		}
		else
		{
			// imperfection
			incidentPoint = orgPoints[triVerts[0]].first;
		}

		std::vector<uint32_t> incidentTries;
		auto referenceTri = contactInfo.bestMinkowskiTriangle;

		// cut incidentTries
		// center of edges
		// contactPoint = 
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
	MinkowskiDiff GetMinkowskiDiff(const Vector3f& direction, const MeshModel& meshA, const MeshModel& meshB)
	{
		uint32_t antagonistA = FarthestPoint(direction, meshA.points);
		uint32_t antagonistB = FarthestPoint(-direction, meshB.points);
		return MinkowskiDiff{ meshA.points[antagonistA] - meshB.points[antagonistB], { antagonistA, antagonistB } };
	}

	uint32_t FarthestPoint(const Vector3f& direction, const std::vector<Vector3f>& points)
	{
		auto farthest = std::make_pair(-(std::numeric_limits<float>::max)(), uint32_t(0));
		for (uint32_t i = 0; i < points.size(); ++i) {
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

private:
	VulkanContext& vulkanContext;
	std::unordered_set<std::shared_ptr<MeshObject>> Renderers;
};
