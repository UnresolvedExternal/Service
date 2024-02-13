#pragma once

#include <format>
#include <Service/Common/Delegate.h>

namespace Service
{
	template <typename T>
	class ActiveValue
	{
	public:
		inline ActiveValue();
		inline ActiveValue(const T& value);
		inline ActiveValue(ActiveValue&& other);
		inline ActiveValue& operator=(ActiveValue&& other) = default;
		inline ActiveValue& operator=(const T& value);

		inline operator const T& () const;
		inline const T& operator*() const;
		inline const T* operator->() const;

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
		inline constexpr auto parse(format_parse_context& context);
		inline auto format(const Service::ActiveValue<T>& value, format_context& context) const;
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
	ActiveValue<T>::ActiveValue(const T& value) :
		value{ value }
	{
	
	}

	template <typename T>
	ActiveValue<T>& ActiveValue<T>::operator=(const T& value)
	{
		const bool changed = this->value != value;
		this->value = value;
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
