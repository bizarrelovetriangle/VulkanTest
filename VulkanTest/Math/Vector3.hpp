#pragma once
#include <cmath>
#include <memory>

template <class T>
class Vector3;
using Vector3f = Vector3<float>;
using Vector3i = Vector3<int>;

template <class T>
class Vector3
{
public:
	T x = 0;
	T y = 0;
	T z = 0;

	Vector3()
	{
	}

	Vector3(T x, T y, T z)
		: x(x), y(y), z(z)
	{
	}

	template <class O>
	Vector3(const Vector3<O>& vec)
		: x(vec.x), y(vec.y), z(vec.z)
	{
	}

	template <class Dummy = bool, std::enable_if_t<std::is_same_v<T, float>, Dummy> = true>
	T Dot(const Vector3<T>& vec) const
	{
		return x * vec.x + y * vec.y + z * vec.z;
	}

	template <class Dummy = bool, std::enable_if_t<std::is_same_v<T, float>, Dummy> = true>
	Vector3<T> Cross(const Vector3<T>& vec) const
	{
		return Vector3<T>(
			z * vec.y - y * vec.z,
			x * vec.z - z * vec.x,
			y * vec.x - x * vec.y);
	}

	Vector3<T> operator*(T s)
	{
		return Vector3<T>(x * s, y * s, z * s);
	}

	Vector3<T> operator/(T s)
	{
		return Vector3<T>(x / s, y / s, z / s);
	}

	Vector3<T> Normalized()
	{
		float length = Length();
		if (length == 0) return Vector3<T>(1., 0., 0.);
		return *this / Length();
	}

	T Length()
	{
		return std::sqrt(Length2());
	}

	T Length2()
	{
		return std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2);
	}
};
