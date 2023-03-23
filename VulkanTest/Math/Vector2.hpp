#pragma once;

template <class T>
class Vector2;
using Vector2f = Vector2<float>;
using Vector2i = Vector2<int>;

template <class T>
class Vector2
{
public:
	T x = 0;
	T y = 0;

	Vector2()
	{
	}

	Vector2(T x, T y)
		: x(x), y(y)
	{
	}
};