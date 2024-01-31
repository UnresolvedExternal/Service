#pragma once

#include <format>
#include <Service/Common/Delegate.h>

namespace Service
{
	template <typename T>
	class ActiveValue
	{
	public:
		ActiveValue();
		ActiveValue(T&& value);
		ActiveValue(ActiveValue&& other);
		ActiveValue& operator=(ActiveValue&& other) = default;
		ActiveValue& operator=(T&& value);

		operator const T& () const;
		const T& operator*() const;
		const T* operator->() const;

		DelegateList<void()> OnChange;

	private:
		T value;
	};
}

#if __cplusplus > 202002L

namespace std
{
	template <typename T>
	struct formatter<Service::ActiveValue<T>> : private formatter<T>
	{
		constexpr auto parse(format_parse_context& context);
		auto format(const Service::ActiveValue<T>& value, format_context& context) const;
	};
}

#endif

namespace Service
{
	template <typename T>
	ActiveValue<T>::ActiveValue() : 
		value{}
	{

	}

	template <typename T> 
	ActiveValue<T>::ActiveValue(T&& value) :
		value{ std::forward<T>(value) }
	{
	
	}

	template <typename T>
	ActiveValue<T>& ActiveValue<T>::operator=(T&& value)
	{
		const bool changed = this->value != value;
		this->value = std::forward<T>(value);
		if (changed) OnChange();
		return *this;
	}

	template <typename T> 
	ActiveValue<T>::operator const T&() const
	{
		return value;
	}

	template <typename T>
	const T& ActiveValue<T>::operator*() const
	{
		return value;
	}

	template <typename T>
	const T* ActiveValue<T>::operator->() const
	{
		return &value;
	}
}

#if __cplusplus > 202002L

namespace std
{
	template <typename T>
	constexpr auto formatter<Service::ActiveValue<T>>::parse(format_parse_context& context)
	{
		return formatter<T>::parse(context);
	}

	template <typename T>
	auto formatter<Service::ActiveValue<T>>::format(const Service::ActiveValue<T>& value, format_context& context) const
	{
		return formatter<T>::format(*value, context);
	}
}

#endif
