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
	//return i * vec.x + j * vec.y + k * vec.z + l * vec.w;

	return {
		i.x * vec.x + j.x * vec.y + k.x * vec.z + l.x * vec.w,
		i.y * vec.x + j.y * vec.y + k.y * vec.z + l.y * vec.w,
		i.z * vec.x + j.z * vec.y + k.z * vec.z + l.z * vec.w,
		i.w * vec.x + j.w * vec.y + k.w * vec.z + l.w * vec.w,
	};
}

Matrix4 Matrix4::operator*(const Matrix4& mat) const
{
	return Matrix4(*this * mat.i, *this * mat.j, *this * mat.k, *this * mat.l);
}

Matrix4 Matrix4::Translate(const Vector3f& vec)
{
	Matrix4 matrix;
	matrix.l.x = vec.x;
	matrix.l.y = vec.y;
	matrix.l.z = vec.z;
	return matrix;
}

Matrix4 Matrix4::LookAt(const Vector3f& from, const Vector3f& to)
{
	auto k = (to - from).Normalized();
	auto [i, j] = k.twoPerpendiculars(); // j should be in one plane with k and z origin
	j = -j;
	auto rotate = Matrix4(Vector4f(i, 1.), Vector4f(j, 1.), Vector4f(k, 1.), Vector4f(0., 0., 0., 1.));
	return Translate(from) * rotate;
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
	return view.Transpose();
}


Matrix4 Matrix4::Rotate(const Vector4f& quaternion)
{
	auto angle = std::acos(quaternion.w);

	if (angle == 0) return Matrix4();

	auto jA = Vector3f(quaternion.x, quaternion.y, quaternion.z) / std::sin(angle);
	auto [iA, kA] = jA.twoPerpendiculars();

	Matrix4 asixRotate{
		{iA.x, jA.x, kA.x, 0.},
		{iA.y, jA.y, kA.y, 0.},
		{iA.z, jA.z, kA.z, 0.},
		{  0.,   0.,   0., 1.} };

	asixRotate = asixRotate.Transpose();

	auto yRotate = RotateY(angle * 2);
	auto matrix = asixRotate * yRotate * asixRotate.Transpose();
	return matrix;
}

Matrix4 Matrix4::Rotate(const Vector3f& from, const Vector3f& to)
{
	auto fromN = from.Normalized();
	auto toN = (from + to).Normalized(); // quaternion takes only half of the angle
	auto quaternion = Vector4f(fromN.Cross(toN), fromN.Dot(toN));
	return Rotate(quaternion);
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

	return (Matrix4{
		{  1.,  0.,  0.,  0.},
		{  0., cos,-sin,  0.},
		{  0., sin, cos,  0.},
		{  0.,  0.,  0.,  1.} }).Transpose();
}

Matrix4 Matrix4::RotateY(float radians)
{
	float cos = std::cos(radians);
	float sin = std::sin(radians);

	return (Matrix4{
		{ cos,  0.,-sin,  0.},
		{  0.,  1.,  0.,  0.},
		{ sin,  0., cos,  0.},
		{  0.,  0.,  0.,  1.} }).Transpose();
}

Matrix4 Matrix4::RotateZ(float radians)
{
	float cos = std::cos(radians);
	float sin = std::sin(radians);
	 
	return (Matrix4{
		{ cos,-sin,  0.,  0.},
		{ sin, cos,  0.,  0.},
		{  0.,  0.,  1.,  0.},
		{  0.,  0.,  0.,  1.} }).Transpose();
}

// https://github.com/nigels-com/glt/blob/master/src/math/matrix4.cpp
Matrix4 Matrix4::Inverse() const
{
	/* pre-compute 2x2 dets for last two rows when computing */
	/* cofactors of first two rows. */
	float d12 = k.x * l.y - l.x * k.y;
	float d13 = k.x * l.z - l.x * k.z;
	float d23 = k.y * l.z - l.y * k.z;
	float d24 = k.y * l.w - l.y * k.w;
	float d34 = k.z * l.w - l.z * k.w;
	float d41 = k.w * l.x - l.w * k.x;

	Vector4f cross4;
	cross4.x =  (j.y * d34 - j.z * d24 + j.w * d23);
	cross4.y = -(j.x * d34 + j.z * d41 + j.w * d13);
	cross4.z =  (j.x * d24 + j.y * d41 + j.w * d12);
	cross4.w = -(j.x * d23 - j.y * d13 + j.z * d12);

	float det = i.x * cross4.x + i.y * cross4.y + i.z * cross4.z + i.w * cross4.w;

	if (det == 0.0) {
		throw std::exception("invert_matrix: Warning: Singular matrix");
	}

	double invDet = 1.0 / det;

	Matrix4 res;
	res.i.x =  (j.y * d34 - j.z * d24 + j.w * d23) * invDet;
	res.i.y = -(j.x * d34 + j.z * d41 + j.w * d13) * invDet;
	res.i.z =  (j.x * d24 + j.y * d41 + j.w * d12) * invDet;
	res.i.w = -(j.x * d23 - j.y * d13 + j.z * d12) * invDet;

	res.j.x = -(i.y * d34 - i.z * d24 + i.w * d23) * invDet;
	res.j.y =  (i.x * d34 + i.z * d41 + i.w * d13) * invDet;
	res.j.z = -(i.x * d24 + i.y * d41 + i.w * d12) * invDet;
	res.j.w =  (i.x * d23 - i.y * d13 + i.z * d12) * invDet;

	/* Pre-compute 2x2 dets for first two rows when computing */
	/* cofactors of last two rows. */
	d12 = i.x * j.y - j.x * i.y;
	d13 = i.x * j.z - j.x * i.z;
	d23 = i.y * j.z - j.y * i.z;
	d24 = i.y * j.w - j.y * i.w;
	d34 = i.z * j.w - j.z * i.w;
	d41 = i.w * j.x - j.w * i.x;

	res.k.x =  (l.y * d34 - l.z * d24 + l.w * d23) * invDet;
	res.k.y = -(l.x * d34 + l.z * d41 + l.w * d13) * invDet;
	res.k.z =  (l.x * d24 + l.y * d41 + l.w * d12) * invDet;
	res.k.w = -(l.x * d23 - l.y * d13 + l.z * d12) * invDet;

	res.l.x = -(k.y * d34 - k.z * d24 + k.w * d23) * invDet;
	res.l.y =  (k.x * d34 + k.z * d41 + k.w * d13) * invDet;
	res.l.z = -(k.x * d24 + k.y * d41 + k.w * d12) * invDet;
	res.l.w =  (k.x * d23 - k.y * d13 + k.z * d12) * invDet;

	return res.Transpose();
}

