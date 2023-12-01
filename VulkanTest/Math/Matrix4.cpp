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
	auto k = (to - from).Normalized();
	auto [i, j] = k.twoPerpendicularsForK(); // j should be in one plane with k and z origin
	auto rotate = Matrix4(Vector4f(i), Vector4f(j), Vector4f(k), Vector4f(0., 0., 0., 1.)).Transpose();
	return Translation(from) * rotate;
}

Matrix4 Matrix4::Frustum(float minDepth, float maxDepth, float angle)
{
	// First of all we want to set the k.w to 1, and 'l.w' to 0
	// - so the W component of result vector will always be equal to the initial Z.
	// By witch all components of the result vector will be eventualy divided

	// Now we have to alter Z component so its range (minDepth, maxDepth) turns into the range (0, 1)
	// We would like to use something like: (Z - minDepth) / (maxDepth - minDepth)
	// Where the 'Z' is a 'depending part' and the '- minDepth' a constant part of expresion
	// But we can't fit this formula in a matrix so we will devise something different.

	// The 'l.z' will be multiplied by 1 and then eventualy divided by Z - so it's directly depende by Z.
	// Unfortunetly this means that the bigger Z the less 'l.z' and vise vesta,
	// but we can play this around by negating this value.
	// ### So we can have our 'depending part' as: -(minDepth * maxDepth) / Z
	// If Z = minDepth the result is '-maxDepth', if Z = maxDepth the result is '-minDepth'
	// And the range (minDepth, maxDepth) turns into the (-maxDepth, -minDepth)

	// Because 'k.z' will multiblied by Z and then eventualy devided by Z - this is basicaly a constant
	// Knowing resulting range of our 'depending part' we can state:
	// ### 'constant part' just as: 'maxDepth'
	// So the 'constant part' + 'depending part' give us the range (0, maxDepth - minDepth)
	// And now we only have to devide it by (maxDepth - minDepth)
	// ### (maxDepth - (minDepth * maxDepth) / Z) / (maxDepth - minDepth)

	float constPart = maxDepth / (maxDepth - minDepth);
	float depenPart = - (minDepth * maxDepth) / (maxDepth - minDepth);

	Matrix4 view{
	    //       i         j         k         l
		{       1.,       0.,       0.,       0.},
		{       0.,       1.,       0.,       0.},
		{       0.,       0.,constPart,depenPart},
		{       0.,       0.,       1.,       0.}};
	return view;
}


Matrix4 Matrix4::Rotate(const Vector4f& quaternion)
{
	auto angle = std::acos(quaternion.w);

	auto jA = Vector3f(quaternion.x, quaternion.y, quaternion.z) / std::sin(angle);
	auto [iA, kA] = jA.twoPerpendicularsForJ(); // Why is it matter?

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

