#pragma once

#include <functional>
#include <Union/String.h>

using Union::StringANSI;

namespace std
{
	template <>
	struct hash<StringANSI> : private hash<string_view>
	{
		size_t operator()(const StringANSI& x) const;
	};

	template <>
	struct equal_to<StringANSI>
	{
		bool operator()(const StringANSI& x, const StringANSI& y) const;
	};

	template <>
	struct less<StringANSI>
	{
		bool operator()(const StringANSI& x, const StringANSI& y) const;
	};

#if __cplusplus > 202002L

	template <>
	struct formatter<StringANSI> : public formatter<string_view>
	{
		auto format(const StringANSI& text, format_context& context) const;
	};

#endif
}

namespace Union
{
	std::weak_ordering operator<=>(const StringANSI& left, const StringANSI& right);
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
		return x.GetDifference(y, Union::StringBase::Flags::IgnoreCase);
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
}
