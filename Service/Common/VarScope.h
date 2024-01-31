#pragma once

namespace Service
{
	// Restores the variable value on destruction
	template <typename T>
	struct VarScope
	{
	public:
		VarScope();
		VarScope(T& var);
		VarScope(T& var, T&& value);
		VarScope(VarScope&& scope);
		VarScope& operator=(VarScope&& right);
		void Restore();
		void Disable();
		~VarScope();

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
	VarScope<T>::VarScope(T& var, T&& value) :
		VarScope{ var }
	{
		var = std::forward<T>(value);
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
	void VarScope<T>::Restore()
	{
		*var = value;
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
