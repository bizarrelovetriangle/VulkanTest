#pragma once
#include <memory>

template <class T>
class Vector4;
using Vector4f = Vector4<float>;

template <class T>
class Vector4
{
public:
	T x = 0;
	T y = 0;
	T z = 0;
	T w = 0;

	Vector4()
	{
	}

	template <class O>
	Vector4(const Vector4<O>& vec)
		: x(vec.x), y(vec.y), z(vec.z), w(vec.w)
	{
	}

	Vector4(T x, T y, T z, T w)
		: x(x), y(y), z(z), w(w)
	{
	}

	template <class Dummy = bool, std::enable_if_t<std::is_same_v<T, float>, Dummy> = true>
	T Dot(const Vector4<T> vec) const
	{
		return x * vec.x + y * vec.y + z * vec.z + w * vec.w;
	}

	void operator*=(T s)
	{
		x *= s;
		y *= s;
		z *= s;
		w *= s;
	}

	template <class T>
	static Vector4<T> FromGLTF(const Vector4<T>& vec)
	{
		return Vector4<T>(vec.x, vec.y, -vec.z, vec.w);
	}
};
