#pragma once
#include "../Objects/Interfaces/MeshObject.h"
#include "../Objects/Primitives/PlaneObject.h"
#include "../Objects/Primitives/ArrowObject.h"
#include "../Renderers/ColoredRenderer.h"
#include "../Renderers/LinedRenderer.h"
#include "../Renderers/SimpleVertexedRenderer.h"
#include "../Renderers/PlaneRenderer.h"
#include <unordered_set>
#include <set>
#include <optional>
#include <queue>
#include <algorithm>
#include "GeometryFunctions.h"
#include "GeometryCreator.h"
#include <numeric>
#include <unordered_map>

struct ContactInfo
{
	bool contact = false;
	std::shared_ptr<MeshModel> minkowskiTriangular;
	std::vector<std::pair<uint32_t, uint32_t>> minkowskiOrgPointPairs;
	std::pair<std::shared_ptr<MeshObject>, std::shared_ptr<MeshObject>> objectPair;
	uint32_t bestMinkowskiTriangle = 0;
	float penetration;
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

		// face the triangle to the zero point
		auto& triPoints = contactInfo.minkowskiTriangular->points;
		if (GeometryFunctions::TrianglePlanePointDist(triPoints[0], triPoints[1], triPoints[2], Vector3f::Zero()) < 0.)
			std::swap(contactInfo.minkowskiTriangular->points[0], contactInfo.minkowskiTriangular->points[2]);

		uint32_t nearestTri = contactInfo.minkowskiTriangular->AddTriangle({ 0, 1, 2 });

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
					contactInfo.minkowskiTriangular->DeleteTriangle(tri);

			// face the triangle from the zero point
			auto nearestTriVerts = contactInfo.minkowskiTriangular->triangles[nearestTri].vertices;
			std::reverse(nearestTriVerts.begin(), nearestTriVerts.end());
			contactInfo.minkowskiTriangular->DeleteTriangle(nearestTri);
			nearestTri = contactInfo.minkowskiTriangular->AddTriangle(nearestTriVerts);

			uint32_t triA = contactInfo.minkowskiTriangular->AddTriangle({ nearestTriVerts[0], newPoint, nearestTriVerts[1] });
			uint32_t triB = contactInfo.minkowskiTriangular->AddTriangle({ nearestTriVerts[1], newPoint, nearestTriVerts[2] });
			uint32_t triC = contactInfo.minkowskiTriangular->AddTriangle({ nearestTriVerts[2], newPoint, nearestTriVerts[0] });

			// check only the newly created triangular pyramid
			if (GeometryFunctions::PointInsideTriangularPyramid(Vector3f::Zero(), *contactInfo.minkowskiTriangular, { triA, triB, triC }))
			{
				contactInfo.contact = true;
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
		bool success = false;
		uint32_t nearestTri = 0;
		std::unordered_map<uint32_t, float> distances;

		for (uint32_t tri = 0; tri < contactInfo.minkowskiTriangular->triangles.size(); ++tri) {
			if (!contactInfo.minkowskiTriangular->triangleBitVector[tri]) continue;
			auto triPoints = contactInfo.minkowskiTriangular->TrianglePoints(tri);
			float dist = -GeometryFunctions::TrianglePlanePointDist(triPoints[0], triPoints[1], triPoints[2], Vector3f::Zero());
			distances.emplace(tri, dist);
		}

		for (uint32_t i = 0; i < 100; ++i)
		{
			nearestTri = std::min_element(distances.begin(), distances.end(), [](auto& a, auto& b) { return a.second < b.second; })->first;

			auto nearestTriPoints = contactInfo.minkowskiTriangular->TrianglePoints(nearestTri);
			auto direction = contactInfo.minkowskiTriangular->TriangleNormal(nearestTri);
			auto minkowskiDiff = GetMinkowskiDiff(direction, meshA, meshB);

			if (minkowskiDiff.diff == nearestTriPoints[0] || minkowskiDiff.diff == nearestTriPoints[1] || minkowskiDiff.diff == nearestTriPoints[2]) {
				success = true;
				break;
			}

			if (GeometryFunctions::TrianglePlanePointDist(nearestTriPoints[0], nearestTriPoints[1], nearestTriPoints[2], minkowskiDiff.diff) < 10e-9) {
				success = false;
				break;
			}

			uint32_t newPoint = contactInfo.minkowskiTriangular->points.size();
			contactInfo.minkowskiTriangular->points.push_back(minkowskiDiff.diff);
			contactInfo.minkowskiOrgPointPairs.push_back(minkowskiDiff.pointPair);

			std::set<std::pair<uint32_t, uint32_t>> contour;
			std::vector<uint32_t> deleteFaces;

			for (auto it = distances.begin(); it != distances.end();) {
				uint32_t tri = it->first;
				auto next = std::next(it);

				auto triPoints = contactInfo.minkowskiTriangular->TrianglePoints(tri);
				if (GeometryFunctions::TrianglePlanePointDist(triPoints[0], triPoints[1], triPoints[2], minkowskiDiff.diff) > 0.) {
					auto& triangle = contactInfo.minkowskiTriangular->triangles[tri];
					for (auto& edge : triangle.edges) {
						auto org = contactInfo.minkowskiTriangular->Origin(edge);
						auto dest = contactInfo.minkowskiTriangular->Destination(edge);

						if (auto it = contour.find(std::make_pair(dest, org)); it != contour.end()) {
							contour.erase(it);
						}
						else {
							contour.emplace(org, dest);
						}
					}

					distances.erase(tri);
					deleteFaces.push_back(tri);
				}

				it = next;
			}

			if (!GeometryFunctions::ContourIsCycled(contour))
			{
				for (auto tri : deleteFaces)
					contactInfo.minkowskiTriangular->markedTris[tri] = true;
				CreatePoint(minkowskiDiff.diff);
				break;
			}


			for (auto tri : deleteFaces) {
				contactInfo.minkowskiTriangular->DeleteTriangle(tri);
			}

			for (auto& edge : contour) {
				uint32_t tri = contactInfo.minkowskiTriangular->AddTriangle({ newPoint, edge.first, edge.second });
				auto triPoints = contactInfo.minkowskiTriangular->TrianglePoints(tri);
				float dist = -GeometryFunctions::TrianglePlanePointDist(triPoints[0], triPoints[1], triPoints[2], Vector3f::Zero());
				distances.emplace(tri, dist);
			}
		}

		contactInfo.bestMinkowskiTriangle = nearestTri;
		auto& triangle = contactInfo.minkowskiTriangular->triangles[contactInfo.bestMinkowskiTriangle];
		auto triNorm = contactInfo.minkowskiTriangular->TriangleNormal(contactInfo.bestMinkowskiTriangle);
		contactInfo.penetration = triNorm.Dot(contactInfo.minkowskiTriangular->points[triangle.vertices.front()]);

		for (uint32_t tri = 0; tri < contactInfo.minkowskiTriangular->triangleBitVector.size(); ++tri)
			if (contactInfo.minkowskiTriangular->triangleBitVector[tri])
				if (tri != nearestTri)
					contactInfo.minkowskiTriangular->DeleteTriangle(tri);

		auto mesh = MeshModel(*contactInfo.minkowskiTriangular);
		mesh.markedTris[nearestTri] = true;
		//CreateObject(std::move(mesh), success);
	}

	void Clipping(const MeshModel& meshA, const MeshModel& meshB, ContactInfo& contactInfo)
	{
		auto& orgPoints = contactInfo.minkowskiOrgPointPairs;
		auto& triVerts = contactInfo.minkowskiTriangular->triangles[contactInfo.bestMinkowskiTriangle].vertices;

		bool referenceIsObjectA = false;

		if (orgPoints[triVerts[0]].first == orgPoints[triVerts[1]].first) {
			referenceIsObjectA = false;
		}
		else if (orgPoints[triVerts[0]].first == orgPoints[triVerts[2]].first) {
			referenceIsObjectA = false;
		}
		else if(orgPoints[triVerts[0]].second == orgPoints[triVerts[1]].second) {
			referenceIsObjectA = true;
		}
		else if (orgPoints[triVerts[0]].second == orgPoints[triVerts[2]].second) {
			referenceIsObjectA = true;
		}
		else {
			// we are not sure
			referenceIsObjectA = true;
		}

		// we substracting second object from first one. So the normal initialy from the second to the first object
		contactInfo.normal = referenceIsObjectA
			? contactInfo.minkowskiTriangular->TriangleNormal(contactInfo.bestMinkowskiTriangle)
			: -contactInfo.minkowskiTriangular->TriangleNormal(contactInfo.bestMinkowskiTriangle);

		auto& arbitraryRefPoint = referenceIsObjectA
			? meshA.points[orgPoints[triVerts[0]].first]
			: meshB.points[orgPoints[triVerts[0]].second];
		Plane referencePlane(arbitraryRefPoint, contactInfo.normal);

		auto& incidentMesh = !referenceIsObjectA ? meshA : meshB;
		auto intersections = referencePlane.MeshIntersections(incidentMesh);
		auto intersectionsCenter = std::accumulate(intersections.begin(), intersections.end(), Vector3f()) / intersections.size();

		contactInfo.contactPoint = intersectionsCenter - contactInfo.normal * contactInfo.penetration;

		{
			auto planeObj = std::make_unique<PlaneObject>(vulkanContext, intersectionsCenter, contactInfo.normal);
			auto planeRenderer = (PlaneRenderer*)planeObj->renderer.get();
			planeRenderer->evenPlaneObjectUniform.color = Vector4f(1., 0., 0., 1.);
			planeRenderer->UpdatePlaneUniformBuffer();
			planeObj->scale = planeObj->scale * 0.5;
			renderers.emplace(std::move(planeObj));

			// arrow of anti penetration force
			auto arrow = std::make_unique<ArrowObject>(vulkanContext, contactInfo.contactPoint, contactInfo.normal);
			arrow->scale = arrow->scale * contactInfo.penetration;
			renderers.emplace(std::move(arrow));
		}
	}

	void Render(RenderVisitor& renderVisitor)
	{
		for (auto& renderer : renderers)
			renderer->Render(renderVisitor);
	}

	void Dispose()
	{
		for (auto& renderer : renderers)
			renderer->Dispose();
		renderers.clear();
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
		uint32_t size = points.size();
		for (uint32_t i = 0; i < size; ++i) {
			auto& point = points[i];
			float dist = direction.Dot(point);

			if (farthest.first < dist) {
				farthest = std::make_pair(dist, i);
			}
		}
		return farthest.second;
	}

	void CreateObject(const MeshModel& mesh, bool success = false)
	{
		Vector4f baseColor = success
			? Vector4(0.1, 0.3, 0.1, 1.)
			: Vector4(0.3, 0.1, 0.1, 1.);

		std::vector<Vector4f> triColors(mesh.triangles.size(), baseColor);

		for (uint32_t tri = 0; tri < mesh.markedTris.size(); ++tri) {
			if (mesh.markedTris[tri])
				triColors[tri] = Vector4f(1., 0., 0., 1.);
		}

		auto renderer = std::make_unique<ColoredRenderer>(vulkanContext, triColors, true);
		auto minkowskiObj = std::make_unique<MeshObject>(std::make_unique<MeshModel>(mesh), std::move(renderer));

		minkowskiObj->renderer->propertiesUniform.baseColor = baseColor;
		minkowskiObj->renderer->UpdatePropertiesUniformBuffer();
		minkowskiObj->UpdateVertexBuffer();
		minkowskiObj->interactive = false;
		renderers.insert(std::move(minkowskiObj));
	}

	void CreatePoint(const Vector3f pos)
	{
		auto ico = std::make_unique<MeshObject>(GeometryCreator::CreateIcosphere(0.1, 1), std::make_unique<SimpleVertexedRenderer>(vulkanContext));
		ico->renderer->propertiesUniform.baseColor = Vector4f(1., 0., 1., 1.);
		ico->position = pos;
		ico->renderer->UpdateTransformUniformBuffer();
		ico->renderer->UpdatePropertiesUniformBuffer();
		ico->interactive = false;
		ico->UpdateVertexBuffer();
		renderers.emplace(std::move(ico));
	}

private:
	VulkanContext& vulkanContext;
	std::unordered_set<std::shared_ptr<MeshObject>> renderers;
};
