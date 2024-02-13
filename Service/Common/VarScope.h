#pragma once

namespace Service
{
	// Restores the variable value on destruction
	template <typename T>
	struct VarScope
	{
	public:
		inline VarScope();
		inline VarScope(T& var);

		template <typename E>
			requires std::is_assignable_v<T&, E>
		inline VarScope(T& var, E&& value);

		inline VarScope(VarScope&& scope);
		inline VarScope& operator=(VarScope&& right);
		inline VarScope& Restore();
		inline void Disable();
		inline ~VarScope();

	private:
		T* var;
		T value;
	};

	template <typename T>
	VarScope<T>::VarScope() :
		var{}
	{

	}

	template <typename T>
	VarScope<T>::VarScope(T& var) :
		var{ &var },
		value{ var }
	{

	}

	template <typename T>
	template <typename E>
		requires std::is_assignable_v<T&, E>
	VarScope<T>::VarScope(T& var, E&& value) :
		VarScope{ var }
	{
		var = std::forward<E>(value);
	}

	template <typename T>
	VarScope<T>::VarScope(VarScope&& scope) :
		var{ scope.var },
		value{ std::move(scope.value) }
	{
		scope.var = nullptr;
	}

	template <typename T>
	VarScope<T>& VarScope<T>::operator=(VarScope<T>&& right)
	{
		~VarScope();
		var = right.var;
		value = std::move(right.value);
		right.var = nullptr;
		return *this;
	}

	template <typename T>
	VarScope<T>& VarScope<T>::Restore()
	{
		*var = value;
		return *this;
	}

	template <typename T>
	void VarScope<T>::Disable()
	{
		var = nullptr;
	}

	template <typename T>
	VarScope<T>::~VarScope()
	{
		if (!var) return;
		*var = value;
		var = nullptr;
	}
}
