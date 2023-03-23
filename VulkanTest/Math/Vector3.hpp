#pragma once;

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
};
