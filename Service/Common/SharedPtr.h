#pragma once

namespace Service
{
	// single threaded smart pointer
	template <typename T>
	class SharedPtr
	{
	public:
		SharedPtr();
		SharedPtr(std::nullptr_t);
		SharedPtr(const SharedPtr& y);
		SharedPtr(SharedPtr&& y);
		SharedPtr& operator=(const SharedPtr& y);
		SharedPtr& operator=(SharedPtr&& y);
		~SharedPtr();

		bool operator==(const SharedPtr& y) const;
		bool operator!=(const SharedPtr& y) const;
		T& operator*() const;
		T* operator->() const;
		T* Get() const;

	private:
		SharedPtr(T& data);

		template <class T, class... TArgs>
		friend SharedPtr<T> MakeShared(TArgs&&... args);

		mutable uint32_t* counter;
		T* pointer;
	};

	template <typename T>
	SharedPtr<T>::SharedPtr(T& data) :
		counter{ new uint32_t{ 1 } },
		pointer{ &data }
	{

	}

	template <typename T>
	SharedPtr<T>::SharedPtr() :
		counter{ nullptr },
		pointer{ nullptr }
	{

	}

	template <typename T>
	SharedPtr<T>::SharedPtr(std::nullptr_t) :
		SharedPtr{}
	{

	}

	template <typename T>
	SharedPtr<T>::SharedPtr(const SharedPtr<T>& y) :
		counter{ y.counter },
		pointer{ y.pointer }
	{
		if (counter)
			*counter += 1;
	}

	template <typename T>
	SharedPtr<T>::SharedPtr(SharedPtr<T>&& y) :
		counter{ y.counter },
		pointer{ y.pointer }
	{
		y.counter = nullptr;
		y.pointer = nullptr;
	}

	template <typename T>
	SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr<T>& y)
	{
		if (this == &y)
			return;

		~SharedPtr();

		counter = y.counter;
		pointer = y.pointer;

		if (counter)
			*counter += 1;

		return *this;
	}

	template <typename T>
	SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr&& y)
	{
		if (this == &y)
			return *this;

		counter = y.counter;
		pointer = y.pointer;

		y.counter = nullptr;
		y.pointer = nullptr;
	}

	template <typename T>
	SharedPtr<T>::~SharedPtr<T>()
	{
		if (!counter)
			return;

		*counter -= 1;

		if (*counter == 0)
		{
			delete counter;
			delete pointer;
		}
	}

	template <typename T>
	bool SharedPtr<T>::operator==(const SharedPtr<T>& y) const
	{
		return pointer == y.pointer;
	}

	template <typename T>
	bool SharedPtr<T>::operator!=(const SharedPtr<T>& y) const
	{
		return pointer != y.pointer;
	}

	template <typename T>
	T& SharedPtr<T>::operator*() const
	{
		return *pointer;
	}

	template <typename T>
	T* SharedPtr<T>::operator->() const
	{
		return pointer;
	}

	template <typename T>
	T* SharedPtr<T>::Get() const
	{
		return pointer;
	}

	template <class T, class... TArgs>
	SharedPtr<T> MakeShared(TArgs&&... args)
	{
		T* pointer{ new T{ std::forward<TArgs>(args)... } };
		return SharedPtr{ *pointer };
	}
}
