#pragma once
#include "Math/Matrix4.h"
#include <chrono>

class Camera
{
public:
	Camera()
	{
		worldToView = Matrix4::Translate(Vector3f(0, 0, 5));
		viewToProj = Matrix4::Frustum(0.1, 100, 1);
	}

	void MouseRightDown(bool down)
	{
		mouseRightDown = down;
	}

	void MouseMiddleDown(bool down)
	{
		mouseMiddleDown = down;
	}

	void MouseMoved(Vector2f mousePos)
	{
		mousePosition = mousePos;

		if (mouseRightDown)
		{
			auto offset = mousePosition - prevMousePosition;
			worldToView = Matrix4::Translate({ offset.x, offset.y, 0 }) * worldToView;
		}

		if (mouseMiddleDown)
		{
			auto center4f = worldToView * Vector4f(0., 0., 0., 1.);
			auto center = Vector3f(center4f.x, center4f.y, center4f.z);
			const auto rotateShift = rotatePoint + center;

			auto offset = mousePosition - prevMousePosition;
			worldToView = Matrix4::Translate(-rotateShift) * worldToView;
			worldToView = Matrix4::RotateX(offset.y) * worldToView;
			worldToView = Matrix4::RotateY(offset.x) * worldToView;
			worldToView = Matrix4::Translate(rotateShift) * worldToView;
		}

		prevMousePosition = mousePosition;
	}

	void Scrolled(Vector2f offset)
	{
		offset = offset / 10;
		worldToView = Matrix4::Translate({ offset.x, -offset.y, 0 }) * worldToView;
	}

	void Zoom(float s)
	{
		worldToView = Matrix4::Translate({ 0, 0, s }) * worldToView;
	}

	void UpdateMatrixes()
	{
	}

	Matrix4 worldToView;
	Matrix4 viewToProj;

	Vector3f rotatePoint = Vector3f(0., 0., 0.);

private:
	bool mouseRightDown = false;
	bool mouseMiddleDown = false;
	Vector2f mousePosition;
	Vector2f prevMousePosition;
};
