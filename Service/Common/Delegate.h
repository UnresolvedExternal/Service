#pragma once

#include <vector>
#include <functional>
#include "SharedPtr.h"

namespace Service
{
	// comparable callable object
	template <typename T>
	class Delegate
	{
	public:
		Delegate();

		template <typename TFunc>
		Delegate(TFunc&& function);

		Delegate(const Delegate&) = default;
		Delegate(Delegate&&) = default;
		Delegate& operator=(const Delegate&) = default;
		Delegate& operator=(Delegate&& y) = default;

		bool operator==(const Delegate& right) const;
		bool operator!=(const Delegate& right) const;

		template <class ...TArgs>
		auto operator()(TArgs&& ...args) const;

		const std::function<T>* GetFunction() const;

	private:
		SharedPtr<std::function<T>> function;
	};

	// ordered list of delegates
	template <class T>
	class DelegateList
	{
	public:

		// safe call of all delegates
		// recursion is unsupported
		// removed delegate could not be called
		// added delegate could not be called before the next call
		template <class ...TArgs>
		void operator()(TArgs&& ... args) const;

		DelegateList& operator+=(const Delegate<T>& delegate);
		DelegateList& operator+=(Delegate<T>&& delegate);
		DelegateList& operator-=(const Delegate<T>& delegate);

	private:
		std::vector<Delegate<T>> delegates;
	};

	template <typename T>
	Delegate<T>::Delegate() :
		function{ MakeShared<std::function<T>>(nullptr) }
	{
		std::function<int()> x;
		x();
	}

	template <typename T>
	template <typename TFunc>
	Delegate<T>::Delegate(TFunc&& function) :
		function{ MakeShared<std::function<T>>(std::forward<TFunc>(function)) }
	{

	}

	template <typename T>
	bool Delegate<T>::operator==(const Delegate<T>& right) const
	{
		return function == right.function;
	}

	template <typename T>
	bool Delegate<T>::operator!=(const Delegate<T>& right) const
	{
		return function != right.function;
	}

	template <typename T>
	template <typename ...TArgs>
	auto Delegate<T>::operator()(TArgs&& ...args) const
	{
		return (*function)(std::forward<TArgs>(args)...);
	}

	template <typename T>
	const std::function<T>* Delegate<T>::GetFunction() const
	{
		return function;
	}

	template <typename T>
	template <class ...TArgs>
	void DelegateList<T>::operator()(TArgs&&... args) const
	{
		auto copy{ delegates };

		for (const auto& delegate : copy)
			if (std::find(begin(delegates), end(delegates), delegate) != end(delegates))
				delegate(args...);
	}

	template <typename T>
	DelegateList<T>& DelegateList<T>::operator+=(const Delegate<T>& delegate)
	{
		delegates.emplace_back(delegate);
		return *this;
	}

	template <typename T>
	DelegateList<T>& DelegateList<T>::operator+=(Delegate<T>&& delegate)
	{
		delegates.push_back(std::move(delegate));
		return *this;
	}

	template <typename T>
	DelegateList<T>& DelegateList<T>::operator-=(const Delegate<T>& delegate)
	{
		if (auto it = std::find(begin(delegates), end(delegates), delegate); it != end(delegates))
			delegates.erase(it);

		return *this;
	}
}
