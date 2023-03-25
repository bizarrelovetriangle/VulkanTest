#pragma once;
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

	Vector4(T x, T y, T z, T w)
		: x(x), y(y), z(z), w(w)
	{
	}

	template <std::enable_if_t<std::is_same_v<T, float>, int> = 0>
	T Dot(const Vector4<T> vec) const
	{
		return x * vec.x + y * vec.y + z * vec.z + w * vec.w;
	}
};
