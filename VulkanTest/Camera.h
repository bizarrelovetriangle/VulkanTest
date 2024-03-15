#pragma once
#include "Math/Matrix4.h"
#include <chrono>

class Camera
{
public:
	Camera()
	{
		view = Matrix4::Translation(Vector3f(0, 0, 5));
		proj = Matrix4::Frustum(0.1, 10, 1);
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
			offset = offset / 1000;
			view = Matrix4::Translation({ offset.x, offset.y, 0 }) * view;
		}

		if (mouseMiddleDown)
		{
			const auto rotateShift = Vector3f(0., 0., -5.);
			auto offset = mousePosition - prevMousePosition;
			offset = offset / 1000;
			view = Matrix4::Translation(rotateShift) * view;
			view = Matrix4::RotateY(offset.x) * view;
			view = Matrix4::RotateX(offset.y) * view;
			view = Matrix4::Translation(-rotateShift) * view;
		}

		prevMousePosition = mousePosition;
	}

	void Scrolled(Vector2f offset)
	{
		offset = offset / 10;
		view = Matrix4::Translation({ offset.x, -offset.y, 0 }) * view;
	}

	void Zoom(float s)
	{
		view = Matrix4::Translation({ 0, 0, s }) * view;
	}

	void UpdateMatrixes()
	{
	}

	Matrix4 view;
	Matrix4 proj;

private:
	bool mouseRightDown = false;
	bool mouseMiddleDown = false;
	Vector2f mousePosition;
	Vector2f prevMousePosition;
};
