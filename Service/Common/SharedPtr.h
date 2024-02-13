#pragma once

namespace Service
{
	// Single threaded smart pointer
	template <typename T>
	class SharedPtr
	{
	public:
		inline SharedPtr();
		inline SharedPtr(std::nullptr_t);
		inline SharedPtr(const SharedPtr& y);
		inline SharedPtr(SharedPtr&& y);
		inline SharedPtr& operator=(const SharedPtr& y);
		inline SharedPtr& operator=(SharedPtr&& y);
		inline ~SharedPtr();

		inline bool operator==(const SharedPtr& y) const;
		inline bool operator!=(const SharedPtr& y) const;
		inline T& operator*() const;
		inline T* operator->() const;
		inline explicit operator bool() const;

		inline T* Get() const;

	private:
		inline SharedPtr(T& data);

		template <class T, class... TArgs>
		friend inline SharedPtr<T> MakeShared(TArgs&&... args);

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
			return *this;

		this->~SharedPtr();

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

		return *this;
	}

	template <typename T>
	SharedPtr<T>::~SharedPtr()
	{
		if (!counter) return;
		*counter -= 1;
		if (counter != 0) return;
		delete counter;
		delete pointer;
		counter = nullptr;
		pointer = nullptr;
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
	SharedPtr<T>::operator bool() const
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
