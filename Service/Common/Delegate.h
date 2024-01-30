#pragma once

#include <vector>
#include <unordered_set>
#include <functional>
#include <Service/Common/SharedPtr.h>

namespace Service
{
	struct DelegateBase {};

	template <typename T>
	concept Callable = std::is_convertible_v<T, std::function<T>>;

	template <typename T>
	concept ReturnsVoid = std::is_same_v<typename std::function<T>::result_type, void>;

	template <typename T>
	concept NonDelegate = !std::is_base_of_v<DelegateBase, std::decay_t<T>>;

	// Comparable callable object
	template <Callable T>
	class Delegate : public DelegateBase
	{
	public:
		Delegate();

		template <NonDelegate TFunc>
		Delegate(TFunc&& function);

		Delegate(const Delegate&) = default;
		Delegate(Delegate&&) = default;
		Delegate& operator=(const Delegate&) = default;
		Delegate& operator=(Delegate&& y) = default;

		bool operator==(const Delegate& right) const;
		bool operator!=(const Delegate& right) const;

		template <class ...TArgs>
		typename std::function<T>::result_type operator()(TArgs&& ...args) const;

		const std::function<T>* GetFunction() const;

	private:
		SharedPtr<std::function<T>> function;
	};

	// Ordered list of delegates
	// MUST NOT contain dublicates
	// MUST NOT do recursive calls
	// Added/removed during the call delegates are not called
	template <ReturnsVoid T>
	class DelegateList
	{
	public:
		template <class ...TArgs>
		void operator()(TArgs&& ... args) const;

		DelegateList& operator+=(const Delegate<T>& delegate);
		DelegateList& operator+=(Delegate<T>&& delegate);
		DelegateList& operator-=(const Delegate<T>& delegate);

	private:
		std::vector<Delegate<T>> delegates;
		std::unordered_set<const std::function<T>*> set;
	};

	template <Callable T>
	Delegate<T>::Delegate() :
		function{ MakeShared<std::function<T>>(nullptr) }
	{

	}

	template <Callable T>
	template <NonDelegate TFunc>
	Delegate<T>::Delegate(TFunc&& function) :
		function{ MakeShared<std::function<T>>(std::forward<TFunc>(function)) }
	{

	}

	template <Callable T>
	bool Delegate<T>::operator==(const Delegate<T>& right) const
	{
		return function == right.function;
	}

	template <Callable T>
	bool Delegate<T>::operator!=(const Delegate<T>& right) const
	{
		return function != right.function;
	}

	template <Callable T>
	template <typename ...TArgs>
	typename std::function<T>::result_type Delegate<T>::operator()(TArgs&& ...args) const
	{
		return (*function)(std::forward<TArgs>(args)...);
	}

	template <Callable T>
	const std::function<T>* Delegate<T>::GetFunction() const
	{
		return function.Get();
	}

	template <ReturnsVoid T>
	template <class ...TArgs>
	void DelegateList<T>::operator()(TArgs&&... args) const
	{
		auto copy{ delegates };

		for (const auto& delegate : copy)
			if (set.find(delegate.GetFunction()) != end(set))
				delegate(args...);
	}

	template <ReturnsVoid T>
	DelegateList<T>& DelegateList<T>::operator+=(const Delegate<T>& delegate)
	{
		set.insert(delegate.GetFunction());
		delegates.emplace_back(delegate);
		return *this;
	}

	template <ReturnsVoid T>
	DelegateList<T>& DelegateList<T>::operator+=(Delegate<T>&& delegate)
	{
		set.insert(delegate.GetFunction());
		delegates.push_back(std::move(delegate));
		return *this;
	}

	template <ReturnsVoid T>
	DelegateList<T>& DelegateList<T>::operator-=(const Delegate<T>& delegate)
	{
		if (auto it = set.find(delegate.GetFunction()); it != end(set))
		{
			set.erase(it);
			delegates.erase(std::find(begin(delegates), end(delegates), delegate));
		}

		return *this;
	}
}
