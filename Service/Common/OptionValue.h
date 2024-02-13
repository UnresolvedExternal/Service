#pragma once

namespace Service
{
	template <typename T>
	class OptionValue
	{
	public:
		inline OptionValue(const T& value);
		inline OptionValue(T&& value = T{});
		inline OptionValue(const OptionValue& other) = default;
		inline OptionValue(OptionValue&& other) = default;
		inline OptionValue& operator=(const OptionValue& other) = default;
		inline OptionValue& operator=(OptionValue&&) = default;

		template <typename E>
		inline const auto& operator[](const E& index) const;

		template <typename E>
		inline auto& operator[](const E& index);

		inline explicit operator T() const;
		inline const T& operator*() const;
		inline T& operator*();
		inline const T* operator->() const;
		inline T* operator->();

	private:
		T value;
	};

	template <typename T>
	bool operator==(const OptionValue<T>& left, const OptionValue<T>& right);

	template <typename T>
	std::istream& operator>>(std::istream& in, OptionValue<T>& value);

	template <typename T>
	std::ostream& operator<<(std::ostream& out, const OptionValue<T>& value);

	template <typename T, typename TAlloc>
	bool operator==(const OptionValue<std::vector<T, TAlloc>>& left, const OptionValue<std::vector<T, TAlloc>>& right);

	template <typename T, typename TAlloc>
	std::istream& operator>>(std::istream& in, OptionValue<std::vector<T, TAlloc>>& value);

	template <typename T, typename TAlloc>
	std::ostream& operator<<(std::ostream& out, const OptionValue<std::vector<T, TAlloc>>& value);
}

namespace Service
{
	template <typename T>
	OptionValue<T>::OptionValue(const T& value) :
		value{ value }
	{

	}

	template <typename T>
	OptionValue<T>::OptionValue(T&& value) :
		value{ std::forward<T>(value) }
	{

	}

	template <typename T>
	template <typename E>
	const auto& OptionValue<T>::operator[](const E& index) const
	{
		return value[index];
	}

	template <typename T>
	template <typename E>
	auto& OptionValue<T>::operator[](const E& index)
	{
		return value[index];
	}

	template <typename T>
	OptionValue<T>::operator T() const
	{
		return value;
	}

	template <typename T>
	const T& OptionValue<T>::operator*() const
	{
		return value;
	}

	template <typename T>
	T& OptionValue<T>::operator*()
	{
		return value;
	}

	template <typename T>
	const T* OptionValue<T>::operator->() const
	{
		return &value;
	}

	template <typename T>
	T* OptionValue<T>::operator->()
	{
		return &value;
	}

	template <typename T>
	bool operator==(const OptionValue<T>& left, const OptionValue<T>& right)
	{
		return *left == *right;
	}

	template <typename T>
	std::istream& operator>>(std::istream& in, OptionValue<T>& value)
	{
		T newValue{};
		in >> newValue;
		value = newValue;
		return in;
	}

	template <typename T>
	std::ostream& operator<<(std::ostream& out, const OptionValue<T>& value)
	{
		return out << *value;
	}

	template <typename T, typename TAlloc>
	bool operator==(const OptionValue<std::vector<T, TAlloc>>& left, const OptionValue<std::vector<T, TAlloc>>& right)
	{
		if (left->size() != right->size())
			return false;

		for (size_t i = 0; i < left->size(); i++)
			if (left[i] != right[i])
				return false;

		return true;
	}

	template <typename T, typename TAlloc>
	std::istream& operator>>(std::istream& in, OptionValue<std::vector<T, TAlloc>>& value)
	{
		value->clear();
		T element{};

		while (in >> element)
			value->push_back(element);

		return in;
	}

	template <typename T, typename TAlloc>
	std::ostream& operator<<(std::ostream& out, const OptionValue<std::vector<T, TAlloc>>& value)
	{
		for (size_t i = 0; i < value->size(); i++)
		{
			if (i) out << " ";
			out << value[i];
		}

		return out;
	}
}
