#pragma once

namespace Service
{
	// Single threaded smart pointer
	template <typename T>
	class SharedPtr
	{
	public:
		inline SharedPtr() noexcept;
		inline SharedPtr(std::nullptr_t) noexcept;
		inline SharedPtr(const SharedPtr& y) noexcept;
		inline SharedPtr(SharedPtr&& y) noexcept;
		inline SharedPtr& operator=(const SharedPtr& y) noexcept;
		inline SharedPtr& operator=(SharedPtr&& y) noexcept;
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
	SharedPtr<T>::SharedPtr() noexcept :
		counter{},
		pointer{}
	{

	}

	template <typename T>
	SharedPtr<T>::SharedPtr(std::nullptr_t) noexcept :
		SharedPtr{}
	{

	}

	template <typename T>
	SharedPtr<T>::SharedPtr(const SharedPtr<T>& y) noexcept :
		counter{ y.counter },
		pointer{ y.pointer }
	{
		if (counter)
			*counter += 1;
	}

	template <typename T>
	SharedPtr<T>::SharedPtr(SharedPtr<T>&& y) noexcept :
		counter{ y.counter },
		pointer{ y.pointer }
	{
		y.counter = nullptr;
		y.pointer = nullptr;
	}

	template <typename T>
	SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr<T>& y) noexcept
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
	SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr&& y) noexcept
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
