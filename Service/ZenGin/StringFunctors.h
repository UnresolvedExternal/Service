#include <functional>

#pragma push_macro("ZSTRING")
#define ZSTRING ::GOTHIC_NAMESPACE::zSTRING

namespace std
{
	template <>
	struct hash<ZSTRING> : private hash<string_view>
	{
		inline size_t operator()(const ZSTRING& x) const;
	};

	template <>
	struct equal_to<ZSTRING>
	{
		inline bool operator()(const ZSTRING& x, const ZSTRING& y) const;
	};

	template <>
	struct less<ZSTRING>
	{
		inline bool operator()(const ZSTRING& x, const ZSTRING& y) const;
	};

#if __cplusplus > 202002L

	template <>
	struct formatter<ZSTRING> : public formatter<string_view>
	{
		inline auto format(const ZSTRING& text, format_context& context) const;
	};

#endif
}

namespace GOTHIC_NAMESPACE
{
	inline std::weak_ordering operator<=>(const zSTRING& left, const zSTRING& right);
	inline std::weak_ordering operator<=>(const zSTRING& left, const char* right);
	inline std::weak_ordering operator<=>(const zSTRING& left, const StringANSI& right);

	template <size_t N>
	inline std::weak_ordering operator<=>(const zSTRING& left, const char (&right)[N]);
}

namespace std
{
	size_t hash<ZSTRING>::operator()(const ZSTRING& x) const
	{
		ZSTRING copy{ x };
		copy.Upper();
		return hash<string_view>::operator()(string_view{ copy });
	}

	bool equal_to<ZSTRING>::operator()(const ZSTRING& x, const ZSTRING& y) const
	{
		return x == y;
	}

	bool less<ZSTRING>::operator()(const ZSTRING& x, const ZSTRING& y) const
	{
		return ::Union::str_compare_ignore_case(x.ToChar(), y.ToChar()) < 0;
	}

#if __cplusplus > 202002L

	auto formatter<ZSTRING>::format(const ZSTRING& text, format_context& context) const
	{
		return formatter<string_view>::format(string_view{ text }, context);
	}

#endif

}

namespace GOTHIC_NAMESPACE
{
	std::weak_ordering operator<=>(const zSTRING& left, const zSTRING& right)
	{
		const auto diff = ::Union::str_compare_ignore_case(left.ToChar(), right.ToChar());

		if (diff < 0) return std::weak_ordering::less;
		if (diff > 0) return std::weak_ordering::greater;
		return std::weak_ordering::equivalent;
	}

	std::weak_ordering operator<=>(const zSTRING& left, const char* right)
	{
		const auto diff = ::Union::str_compare_ignore_case(left.ToChar(), right);

		if (diff < 0) return std::weak_ordering::less;
		if (diff > 0) return std::weak_ordering::greater;
		return std::weak_ordering::equivalent;
	}

	inline std::weak_ordering operator<=>(const zSTRING& left, const StringANSI& right)
	{
		return left <=> static_cast<const char*>(right);
	}

	template <size_t N>
	inline std::weak_ordering operator<=>(const zSTRING& left, const char (&right)[N])
	{
		return left <=> static_cast<const char*>(right);
	}
}

#pragma pop_macro("ZSTRING")
