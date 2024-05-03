#pragma once
#include "Math/Matrix4.h"
#include <chrono>

class Camera
{
public:
	Camera()
	{
		view = Matrix4::Translate(Vector3f(0, 0, 5));
		proj = Matrix4::Frustum(0.1, 100, 1);
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
			view = Matrix4::Translate({ offset.x, offset.y, 0 }) * view;
		}

		if (mouseMiddleDown)
		{
			auto center4f = view * Vector4f(0., 0., 0., 1.);
			auto center = Vector3f(center4f.x, center4f.y, center4f.z);
			const auto rotateShift = rotatePoint + center;

			auto offset = mousePosition - prevMousePosition;
			view = Matrix4::Translate(-rotateShift) * view;
			view = Matrix4::RotateX(offset.y) * view;
			view = Matrix4::RotateY(offset.x) * view;
			view = Matrix4::Translate(rotateShift) * view;
		}

		prevMousePosition = mousePosition;
	}

	void Scrolled(Vector2f offset)
	{
		offset = offset / 10;
		view = Matrix4::Translate({ offset.x, -offset.y, 0 }) * view;
	}

	void Zoom(float s)
	{
		view = Matrix4::Translate({ 0, 0, s }) * view;
	}

	void UpdateMatrixes()
	{
	}

	Matrix4 view;
	Matrix4 proj;

	Vector3f rotatePoint = Vector3f(0., 0., 0.);

private:
	bool mouseRightDown = false;
	bool mouseMiddleDown = false;
	Vector2f mousePosition;
	Vector2f prevMousePosition;
};
