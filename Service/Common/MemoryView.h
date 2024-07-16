#pragma once

namespace Service
{
	template <typename T>
	class MemoryView
	{
	public:
		template <typename E>
		inline explicit MemoryView(const E& address, bool lockBack = false, bool flush = true);

		inline MemoryView(MemoryView&& other);
		inline MemoryView& operator=(MemoryView&& other);
		inline ~MemoryView();

		inline void operator=(const T& value) const;
		inline T& operator*() const;
		inline T* operator->() const;

		inline int GetAddress() const;
		inline int GetSize() const;
		inline int GetNextAddress() const;

	private:
		T* memory;
		DWORD oldProtection;
		bool lockBack : 1;
		bool flush : 1;
	};
}

#pragma region Implementation

namespace Service
{
	template <typename T>
	template <typename E>
	MemoryView<T>::MemoryView(const E& address, bool lockBack, bool flush) :
		memory{ static_cast<T*>(reinterpret_cast<void*>(address)) },
		lockBack{ lockBack },
		flush{ flush }
	{
		VirtualProtect(memory, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtection);
	}

	template <typename T>
	MemoryView<T>::MemoryView(MemoryView&& other)
	{
		this->~MemoryView();
		memory = other.memory;
		oldProtection = other.oldProtection;
		lockBack = other.lockBack;
		flush = other.flush;
		other.memory = nullptr;
	}

	template <typename T>
	MemoryView<T>& MemoryView<T>::operator=(MemoryView&& other)
	{
		this->~MemoryView();
		memory = other.memory;
		oldProtection = other.oldProtection;
		lockBack = other.lockBack;
		flush = other.flush;
		other.memory = nullptr;
		return *this;
	}

	template <typename T>
	MemoryView<T>::~MemoryView()
	{
		if (!memory)
			return;

		if (lockBack)
		{
			DWORD junk;
			VirtualProtect(memory, sizeof(T), oldProtection, &junk);
		}

		if (flush)
			FlushInstructionCache(GetCurrentProcess(), memory, sizeof(T));

		memory = nullptr;
	}


	template <typename T>
	void MemoryView<T>::operator=(const T& value) const
	{
		*memory = value;
	}

	template <typename T>
	T& MemoryView<T>::operator*() const
	{
		return *memory;
	}

	template <typename T>
	T* MemoryView<T>::operator->() const
	{
		return memory;
	}

	template <typename T>
	int MemoryView<T>::GetAddress() const
	{
		return reinterpret_cast<int>(memory);
	}

	template <typename T>
	int MemoryView<T>::GetSize() const
	{
		return sizeof(T);
	}

	template <typename T>
	int MemoryView<T>::GetNextAddress() const
	{
		return GetAddress() + GetSize();
	}
}

#pragma endregion
