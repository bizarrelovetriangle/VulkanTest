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
	inline static Vector3 Zero() { return Vector3(0., 0., 0.); }

	T x = 0;
	T y = 0;
	T z = 0;

	Vector3()
	{
	}

	Vector3(T s)
		: x(s), y(s), z(s)
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

	template <class Dummy = T, std::enable_if_t<std::is_same_v<Dummy, float>, bool> = true>
	inline T Dot(const Vector3<T>& vec) const noexcept
	{
		return x * vec.x + y * vec.y + z * vec.z;
	}

	template <class Dummy = T, std::enable_if_t<std::is_same_v<Dummy, float>, bool> = true>
	inline Vector3<T> Cross(const Vector3<T>& vec) const noexcept
	{
		return Vector3<T>(
			z * vec.y - y * vec.z,
			x * vec.z - z * vec.x,
			y * vec.x - x * vec.y);
	}

	inline Vector3<T> operator-() const noexcept
	{
		return Vector3<T>(-x, -y, -z);
	}

	inline Vector3<T> operator-(const Vector3<T>& v) const noexcept
	{
		return Vector3<T>(x - v.x, y - v.y, z - v.z);
	}
	
	inline Vector3<T> operator+(const Vector3<T>& v) const noexcept
	{
		return Vector3<T>(x + v.x, y + v.y, z + v.z);
	}

	inline void operator-=(const Vector3<T>& v) noexcept
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
	}

	inline void operator+=(const Vector3<T>& v) noexcept
	{
		x += v.x;
		y += v.y;
		z += v.z;
	}

	inline Vector3<T> operator*(T s) const noexcept
	{
		return Vector3<T>(x * s, y * s, z * s);
	}

	inline Vector3<T> operator/(T s) const noexcept
	{
		return Vector3<T>(x / s, y / s, z / s);
	}

	inline Vector3<T> Normalized() const noexcept
	{
		float length = Length();
		if (length == 0) return Vector3<T>(1., 0., 0.);
		return *this / Length();
	}

	inline T Length() const noexcept
	{
		return std::sqrt(Length2());
	}

	inline T Length2() const noexcept
	{
		return std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2);
	}

	inline friend bool operator==(const Vector3<T>& vecA, const Vector3<T>& vecB) noexcept
	{
		return vecA.x == vecB.x && vecA.y == vecB.y && vecA.z == vecB.z;
	}

	template <class Dummy = bool, std::enable_if_t<std::is_same_v<T, float>, Dummy> = true>
	inline std::pair<Vector3<T>, Vector3<T>> twoPerpendiculars() const noexcept
	{
		Vector3f zPlus(0., 0., 1.);
		Vector3f yPlus(0., 1., 0.);

		auto j = this->Normalized();
		auto ref = j.Cross(zPlus).Length() > 10e-8 ? zPlus : yPlus;
		auto i = j.Cross(ref).Normalized();
		auto k = j.Cross(i);
		return std::make_pair(i, k);
	}

	inline static Vector3<T> FromGLTF(const Vector3<T>& vec) noexcept
	{
		return Vector3<T>(vec.x, vec.y, -vec.z);
	}
};
