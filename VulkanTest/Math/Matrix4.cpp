#pragma once
#include <cmath>
#include "Matrix4.h"

Matrix4::Matrix4()
{};

Matrix4::Matrix4(const Vector4f& i, const Vector4f& j, const Vector4f& k, const Vector4f& l)
	: i(i), j(j), k(k), l(l)
{};

Vector4f Matrix4::operator*(const Vector4f& vec) const
{
	return Vector4f(i.Dot(vec), j.Dot(vec), k.Dot(vec), l.Dot(vec));
}

Matrix4 Matrix4::operator*(const Matrix4& mat) const
{
	auto transposed = mat.Transpose();
	return Matrix4(transposed * i, transposed * j, transposed * k, transposed * l);
}

Matrix4 Matrix4::Translation(const Vector3f& vec)
{
	Matrix4 matrix;
	matrix.i.w = vec.x;
	matrix.j.w = vec.y;
	matrix.k.w = vec.z;
	return matrix;
}

Matrix4 Matrix4::LookAt(const Vector3f& from, const Vector3f& to)
{
	auto z = (to - from).Normalized();
	auto [x, y] = z.twoPerpendicularsForK();
	auto rotate = Matrix4(Vector4f(x), Vector4f(y), Vector4f(z), Vector4f(0., 0., 0., 1.)).Transpose();
	return Translation(from) * rotate;
}

Matrix4 Matrix4::Rotate(const Vector4f& quaternion)
{
	auto angle = std::acos(quaternion.w);

	auto jA = Vector3f(quaternion.x, quaternion.y, quaternion.z) / std::sin(angle);
	auto [iA, kA] = jA.twoPerpendicularsForJ();

	Matrix4 asixRotate{
		{iA.x, jA.x, kA.x, 0.},
		{iA.y, jA.y, kA.y, 0.},
		{iA.z, jA.z, kA.z, 0.},
		{  0.,   0.,   0., 1.} };

	auto yRotate = RotateY(angle * 2);
	auto matrix = asixRotate * yRotate * asixRotate.Transpose();
	return matrix;
}

Matrix4 Matrix4::Scale(const Vector3f& vec)
{
	Matrix4 matrix;
	matrix.i.x = vec.x;
	matrix.j.y = vec.y;
	matrix.k.z = vec.z;
	return matrix;
}

Matrix4 Matrix4::Transpose() const
{
	return Matrix4{
		{i.x, j.x, k.x, l.x},
		{i.y, j.y, k.y, l.y},
		{i.z, j.z, k.z, l.z},
		{i.w, j.w, k.w, l.w} };
}

Matrix4 Matrix4::RotateX(float radians)
{
	float cos = std::cos(radians);
	float sin = std::sin(radians);

	return Matrix4{
		{  1.,  0.,  0.,  0.},
		{  0., cos,-sin,  0.},
		{  0., sin, cos,  0.},
		{  0.,  0.,  0.,  1.} };
}

Matrix4 Matrix4::RotateY(float radians)
{
	float cos = std::cos(radians);
	float sin = std::sin(radians);

	return Matrix4{
		{ cos,  0.,-sin,  0.},
		{  0.,  1.,  0.,  0.},
		{ sin,  0., cos,  0.},
		{  0.,  0.,  0.,  1.} };
}

Matrix4 Matrix4::RotateZ(float radians)
{
	float cos = std::cos(radians);
	float sin = std::sin(radians);

	return Matrix4{
		{ cos,-sin,  0.,  0.},
		{ sin, cos,  0.,  0.},
		{  0.,  0.,  1.,  0.},
		{  0.,  0.,  0.,  1.} };
}

