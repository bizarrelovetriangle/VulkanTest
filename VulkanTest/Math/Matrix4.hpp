#pragma once;
#include "Vector4.hpp"

class Matrix4
{
public:
	Vector4f i = Vector4f(1., 0., 0., 0.);
	Vector4f j = Vector4f(0., 1., 0., 0.);
	Vector4f k = Vector4f(0., 0., 1., 0.);
	Vector4f l = Vector4f(0., 0., 0., 1.);

	Matrix4()
	{};

	Matrix4(const Vector4f& i, const Vector4f& j, const Vector4f& k, const Vector4f& l)
		: i(i), j(j), k(k), l(l)
	{};

	Vector4f operator*(const Vector4f& vec) const
	{
		return Vector4f(i.Dot(vec), j.Dot(vec), k.Dot(vec), l.Dot(vec));
	}

	Matrix4 operator*(const Matrix4& mat) const
	{
		auto transposed = mat.Transpose();
		return Matrix4(transposed * i, transposed * j, transposed * k, transposed * l);
	}

	static Matrix4 Translation(const Vector3f& vec)
	{
		Matrix4 matrix;
		matrix.i.w = vec.x;
		matrix.j.w = vec.y;
		matrix.k.w = vec.z;
		return matrix;
	}

	static Matrix4 Rotate(const Vector4f& quarternion)
	{
		Matrix4 matrix;
		return matrix;
	}

	static Matrix4 Scale(const Vector3f& vec)
	{
		Matrix4 matrix;
		matrix.i.x = vec.x;
		matrix.j.y = vec.y;
		matrix.k.z = vec.z;
		return matrix;
	}

	Matrix4 Transpose() const
	{
		return Matrix4(
			Vector4f(i.x, j.x, k.x, l.x),
			Vector4f(i.y, j.y, k.y, l.y),
			Vector4f(i.z, j.z, k.z, l.z),
			Vector4f(i.w, j.w, k.w, l.w));
	}
};
