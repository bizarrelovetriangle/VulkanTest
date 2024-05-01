#pragma once
#include <cmath>
#include "Vector3.h"
#include "Vector4.h"

class Matrix4
{
public:
	Vector4f i = Vector4f(1., 0., 0., 0.);
	Vector4f j = Vector4f(0., 1., 0., 0.);
	Vector4f k = Vector4f(0., 0., 1., 0.);
	Vector4f l = Vector4f(0., 0., 0., 1.);

	Matrix4();
	Matrix4(const Vector4f& i, const Vector4f& j, const Vector4f& k, const Vector4f& l);
	Vector4f operator*(const Vector4f& vec) const;
	Matrix4 operator*(const Matrix4& mat) const;

	static Matrix4 Translation(const Vector3f& vec);

	static Matrix4 LookAt(const Vector3f& from, const Vector3f& to);

	static Matrix4 Frustum(float minDepth, float maxDepth, float angle);

	static Matrix4 Rotate(const Vector4f& quaternion);

	static Matrix4 Rotate(const Vector3f& from, const Vector3f& to);

	static Matrix4 Scale(const Vector3f& vec);

	Matrix4 Transpose() const;

	static Matrix4 RotateX(float radians);
	static Matrix4 RotateY(float radians);
	static Matrix4 RotateZ(float radians);

	Matrix4 Inverse() const;
};
