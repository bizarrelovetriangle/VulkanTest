#pragma once
#include <exception>

template <class T>
class Disposable
{
public:
	virtual ~Disposable()
	{
		if (!disposed)
		{
			disposed = true;
			static_cast<T*>(this)->DisposeAction();
		}
	}

	virtual void Dispose() final
	{
		if (!disposed)
		{
			disposed = true;
			static_cast<T*>(this)->DisposeAction();
		}
	}

protected:
	bool disposed = false;
};