#pragma once

#include <functional>
#include <Union/String.h>

using Union::StringANSI;

namespace std
{
	template <>
	struct hash<StringANSI> : private hash<string_view>
	{
		inline size_t operator()(const StringANSI& x) const;
	};

	template <>
	struct equal_to<StringANSI>
	{
		inline bool operator()(const StringANSI& x, const StringANSI& y) const;
	};

	template <>
	struct less<StringANSI>
	{
		inline bool operator()(const StringANSI& x, const StringANSI& y) const;
	};

#if __cplusplus > 202002L

	template <>
	struct formatter<StringANSI> : public formatter<string_view>
	{
		inline auto format(const StringANSI& text, format_context& context) const;
	};

#endif
}

namespace Union
{
	inline std::weak_ordering operator<=>(const StringANSI& left, const StringANSI& right);
	inline std::weak_ordering operator<=>(const StringANSI& left, const char* right);
	
	template <size_t N>
	inline std::weak_ordering operator<=>(const StringANSI& left, const char (&right)[N]);
}

namespace std
{
	size_t hash<StringANSI>::operator()(const StringANSI& x) const
	{
		StringANSI copy{ x };
		copy.MakeUpper();
		return hash<string_view>::operator()(string_view{ copy });
	}

	bool equal_to<StringANSI>::operator()(const StringANSI& x, const StringANSI& y) const
	{
		return x.IsSame(y, Union::StringBase::Flags::IgnoreCase);
	}

	bool less<StringANSI>::operator()(const StringANSI& x, const StringANSI& y) const
	{
		return x.GetDifference(y, Union::StringBase::Flags::IgnoreCase) < 0;
	}

#if __cplusplus > 202002L

	auto formatter<StringANSI>::format(const StringANSI& text, format_context& context) const
	{
		return formatter<string_view>::format(string_view{ text }, context);
	}

#endif

}

namespace Union
{
	std::weak_ordering operator<=>(const StringANSI& left, const StringANSI& right)
	{
		const auto diff = left.GetDifference(right, StringBase::Flags::IgnoreCase);

		if (diff < 0) return std::weak_ordering::less;
		if (diff > 0) return std::weak_ordering::greater;
		return std::weak_ordering::equivalent;
	}

	std::weak_ordering operator<=>(const StringANSI& left, const char* right)
	{
		const auto diff = str_compare_ignore_case(left, right);

		if (diff < 0) return std::weak_ordering::less;
		if (diff > 0) return std::weak_ordering::greater;
		return std::weak_ordering::equivalent;
	}

	template <size_t N>
	std::weak_ordering operator<=>(const StringANSI& left, const char (&right)[N])
	{
		return left <=> static_cast<const char*>(right);
	}
}
