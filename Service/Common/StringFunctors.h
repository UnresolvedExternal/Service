#pragma once

#include <functional>
#include <Union/String.h>

using Union::StringANSI;

namespace std
{
	template <>
	struct hash<StringANSI> : hash<string_view>
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

	template <>
	struct formatter<StringANSI> : public formatter<string_view>
	{
		auto format(const StringANSI& text, format_context& context);
	};

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

	auto formatter<StringANSI>::format(const StringANSI& text, format_context& context)
	{
		return formatter<string_view>::format(string_view{ text }, context);
	}
}
