#pragma once
#include "Objects/Interfaces/Object.h"
#include "Objects/Interfaces/MeshObject.h"
#include "Camera.h"

namespace
{
	Vector3f ToVector3(const Vector4f& vec)
	{
		return {vec.x, vec.y, vec.z};
	}
}

class Picker
{
public:
	void Init(VulkanContext& vulkanContext)
	{
		auto boundingBox = BoundingBox();
		boundingBox.aa = { -0.1, -0.1, -0.1 };
		boundingBox.bb = { 0.1, 0.1, 0.1 };

		render = std::make_shared<BoundingBoxObject>(vulkanContext, boundingBox, false);
		boundingBox.renderBoundingBoxObject = render;
	}

	void Update(const std::vector<std::shared_ptr<Object>>& objects, const Camera& camera)
	{
		auto inverseView = camera.view.Inverse();
		auto segmentA = ToVector3(inverseView * Vector4f(Vector3f::Zero(), 1.));
		auto segmentB = ToVector3(inverseView * Vector4f(mouseDirection * 1000, 1.));
		
		float minDist = (std::numeric_limits<float>::max)();
		Vector3f pos = segmentA;

		for (auto& object : objects)
		{
			if (std::dynamic_pointer_cast<BoundingBoxObject>(object)) continue;

			auto meshObject = std::dynamic_pointer_cast<MeshObject>(object);
			if (meshObject)
			{
				auto objMatrix = object->ComposeMatrix();
				auto& mesh = *meshObject->mesh;
				auto& triangleBV = mesh.triangleBitVector;
				for (int i = 0; i < mesh.triangles.size(); ++i)
				{
					if (!triangleBV[i]) continue;
					auto points = mesh.TrianglePoints(i);

					for (auto& point : points)
						point = ToVector3(objMatrix * Vector4f(point, 1.));

					auto triNorm = (points[1] - points[0]).Cross(points[2] - points[0]).Normalized();

					if (triNorm.Dot(segmentB - segmentA) > 0.)
						continue;

					Plane plane(points.front(), triNorm);
					float ratio = 0.;
					Vector3f intersectPoint;
					if (plane.Intersect(segmentA, segmentB, &intersectPoint, &ratio))
					{
						Vector3f a_b_point_vector = (points[1] - points[0]).Cross(intersectPoint - points[0]);
						Vector3f b_c_point_vector = (points[2] - points[1]).Cross(intersectPoint - points[1]);
						Vector3f c_a_point_vector = (points[0] - points[2]).Cross(intersectPoint - points[2]);

						bool inside =
							triNorm.Dot(a_b_point_vector) > 0 &&
							triNorm.Dot(b_c_point_vector) > 0 &&
							triNorm.Dot(c_a_point_vector) > 0;
						
						if (!inside) continue;

						if (ratio < minDist)
						{
							pos = intersectPoint;
							minDist = ratio;
						}
					}
				}
			}
		}

		render->position = pos;
	}

	void MouseMoved(Vector2f mousePos, const Camera& camera)
	{
		mouseDirection = Vector3f(mousePos.x, mousePos.y, 1.).Normalized();
		//render->position = ToVector3(camera.view.Inverse() * Vector4f(mouseDirection * 5, 1));
	}

	std::shared_ptr<Object> pickedObj;
	std::shared_ptr<BoundingBoxObject> render;
	Vector3f mouseDirection;
	Vector3f pickedPos;
};
