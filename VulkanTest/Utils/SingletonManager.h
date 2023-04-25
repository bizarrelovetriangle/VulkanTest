#pragma once
#include <functional>
#include <optional>
#include <memory>

template <class T>
class Singleton
{
public:
	static std::unique_ptr<T> Value;
};

template <class T>
std::unique_ptr<T> Singleton<T>::Value;

class SingletonManager
{
public:
	template <class T>
	T& Create(std::function<std::unique_ptr<T>()> constructor, std::function<void(T&)> destructor)
	{
		if (!Singleton<T>::Value)
		{
			Singleton<T>::Value = constructor();
			destructors.push_back([destructor = std::move(destructor), &value = *Singleton<T>::Value]
				() mutable { destructor(value); });
		}

		return *Singleton<T>::Value;
	}

	template <class T>
	T& Get()
	{
		return *Singleton<T>::Value;
	}

	void Dispose()
	{
		for (auto& destructor : destructors) destructor();
	}

private:
	std::vector<std::function<void()>> destructors;
};
