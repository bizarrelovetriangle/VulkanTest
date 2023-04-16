#pragma once

template <class T>
class Vector2;
using Vector2f = Vector2<float>;
using Vector2i = Vector2<int>;
using Vector2u = Vector2<uint32_t>;

template <class T>
class Vector2
{
public:
	T x = 0;
	T y = 0;

	Vector2()
	{
	}

	template <class O>
	Vector2(const Vector2<O>& vec)
		: x(vec.x), y(vec.y)
	{
	}

	Vector2(T x, T y)
		: x(x), y(y)
	{
	}
};