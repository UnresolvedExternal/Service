#include <tuple>

namespace GOTHIC_NAMESPACE
{

#ifndef __ZEXTERNAL
#define __ZEXTERNAL

#define INTERNALS_EXTERNAL_WRAPPER(cppName, suffix) \
	void __cdecl cppName ## _ ## suffix() \
	{ \
		using Info = Internals::ExternalFunctionInfo<decltype(&cppName)>; \
 \
		[] <std::size_t... Indexes> (std::index_sequence<Indexes...>) \
		{ \
			Info::UnreferencedArguments arguments; \
			zARGS(std::get<Indexes>(arguments)...); \
 \
			if constexpr (::Service::Internals::SameAsAny<Info::ReturnType, void>) \
				cppName(std::get<Indexes>(arguments)...); \
			else \
				zRETURN(cppName(std::get<Indexes>(arguments)...)); \
		}(std::make_index_sequence<std::tuple_size_v<Info::UnreferencedArguments>>{}); \
	}

#define INTERNALS_EXTERNAL_REGISTRATION(parser, cppName, dName, wrapperName) \
	GameSub wrapperName ## _reg(ZTEST(GameEvent::DefineExternals), [] \
	{ \
		using Info = ExternalFunctionInfo<decltype(&cppName)>; \
 \
		[] <std::size_t... Indexes> (std::index_sequence<Indexes...>) \
		{ \
			::GOTHIC_NAMESPACE::Internals::RegisterExternalFunc< \
				std::remove_const_t<std::remove_reference_t<Info::ReturnType>>, \
				std::remove_reference_t<decltype(std::get<Indexes>(Info::UnreferencedArguments{}))>... \
			>(parser, dName, &::GOTHIC_NAMESPACE::Internals::wrapperName); \
		}(std::make_index_sequence<std::tuple_size_v<Info::ArgumentTypes>>{}); \
	});

#define ZEXTERNAL_EX(cppName, parser, dName) \
	namespace Internals \
	{ \
		INTERNALS_EXTERNAL_WRAPPER(cppName, wrapper) \
		INTERNALS_EXTERNAL_REGISTRATION(parser, cppName, dName, cppName ## _wrapper) \
	}

#define ZEXTERNAL(cppName) ZEXTERNAL_EX(cppName, parser, #cppName)
#define ZEXTERNAL_NS(cppName) ZEXTERNAL_EX(cppName, parser, (std::string{ PROJECT_NAME } + ":" + #cppName).c_str())

#endif

	template <ParserType... TArgs>
	inline void zARGS(TArgs&... args);

	template <ParserArgument T>
	inline void zRETURN(const T& value);

	template <ParserType... TArgs>
	inline void PopArguments(zCParser* parser, TArgs&... args);

	template <ParserArgument T>
	inline void SetReturn(zCParser* parser, const T& value);

	template <ParserType TReturn, ParserArgument... TArgs>
	inline TReturn Call(zCParser* parser, const char* function, const TArgs&... args);

	template <ParserType TReturn, ParserArgument... TArgs>
	inline TReturn Call(zCParser* parser, int function, const TArgs&... args);

	namespace Internals
	{
		inline void PopArguments(zCParser* parser);

		template <ParserType T>
		inline void PopArgument(zCParser* parser, T& argument);

		template <ParserType THead, ParserType... TTail>
		inline void PopArguments(zCParser* parser, THead& head, TTail&... tail);

		inline void PushArguments(zCParser* parser, int junk);

		template <ParserArgument T>
		inline void PushArgument(zCParser* parser, int useSymbolIndex, const T& arg);

		template <ParserType THead, ParserType... TTail>
		inline void PushArguments(zCParser* parser, int startSymbolIndex, const THead& head, const TTail&... tail);

		template <typename T>
			requires ParserType<T> || ParserArgument<T>
		inline constexpr int GetParserType();

		template <ParserType TReturn, ParserArgument... TArgs>
		inline void RegisterExternalFunc(zCParser* parser, const char* name, AnyPtr function);

		template <typename T>
		struct ExternalFunctionInfo;

		template <typename TReturn, typename... TArgs>
		struct ExternalFunctionInfo<TReturn(*)(TArgs...)>
		{
			typedef TReturn ReturnType;
			typedef std::tuple<TArgs...> ArgumentTypes;
			typedef std::tuple<std::remove_const_t<std::remove_reference_t<TArgs>>...> UnreferencedArguments;
		};
	}

#pragma region Implementation

	template <ParserType... TArgs>
	void zARGS(TArgs&... args)
	{
		PopArguments(zCParser::GetParser(), args...);
	}

	template <ParserArgument T>
	void zRETURN(const T& value)
	{
		SetReturn(zCParser::GetParser(), value);
	}

	template <ParserType... TArgs>
	void PopArguments(zCParser* parser, TArgs&... args)
	{
		Internals::PopArguments(parser, args...);
	}

	template <ParserArgument T>
	void SetReturn(zCParser* parser, const T& value)
	{
		Internals::PushArguments(parser, -1, value);
	}

	template <ParserType TReturn, ParserArgument... TArgs>
	TReturn Call(zCParser* parser, const char* function, const TArgs&... args)
	{
		return Call<TReturn>(parser, parser->GetIndex(zSTRING{ function }.Upper()), args...);
	}

	template <ParserType TReturn, ParserArgument... TArgs>
	TReturn Call(zCParser* parser, int function, const TArgs&... args)
	{
		Symbol symbol{ parser, function };
		Internals::PushArguments(parser, symbol.GetIndex() + 1, args...);

		if (symbol.Classify() == SymbolType::ExternalFunc)
			reinterpret_cast<int(*)()>(symbol.AsArray<void*>()[0])();
		else
			parser->DoStack(symbol.AsArray<int>()[0]);

		if constexpr (::Service::Internals::SameAsAny<TReturn, void>)
			return;
		else
		{
			TReturn result;
			Internals::PopArguments(parser, result);
			return result;
		}
	}

	namespace Internals
	{
		inline void PopArguments(zCParser* parser)
		{

		}

		template <ParserType T>
		void PopArgument(zCParser* parser, T& arg)
		{
			if constexpr (::Service::Internals::SameAsAny<T, int, bool, unsigned>)
			{
				int data;
				parser->GetParameter(data);
				arg = static_cast<T>(data);
			}
			else if constexpr (::Service::Internals::SameAsAny<T, float, zSTRING>)
				parser->GetParameter(arg);
			else if (zCPar_Symbol* symbol = parser->GetSymbol(parser->datastack.Pop()))
				if constexpr (std::assignable_from<zCObject*&, T>)
					arg = dynamic_cast<T>(reinterpret_cast<zCObject*>(symbol->offset));
				else
					arg = static_cast<T>(reinterpret_cast<void*>(symbol->offset));
			else
				arg = nullptr;
		}

		template <ParserType THead, ParserType... TTail>
		void PopArguments(zCParser* parser, THead& head, TTail&... tail)
		{
			PopArguments(parser, tail...);
			PopArgument(parser, head);
		}

		void PushArguments(zCParser* parser, int junk)
		{

		}

		template <ParserArgument T>
		void PushArgument(zCParser* parser, int useSymbolIndex, const T& arg)
		{
			if constexpr (::Service::Internals::SameAsAny<T, int, bool, unsigned>)
				parser->SetReturn(static_cast<int>(arg));
			else if constexpr (::Service::Internals::SameAsAny<T, float>)
				parser->SetReturn(arg);
			else if constexpr (::Service::Internals::SameAsAny<T, zSTRING, const char*>)
				parser->SetReturn(Internals::StringPool::GetInstance().AddString(arg));
			else if (useSymbolIndex == -1)
				parser->SetReturn(static_cast<void*>(arg));
			else
			{
				parser->GetSymbol(useSymbolIndex)->offset = reinterpret_cast<int>(arg);
				parser->datastack.Push(useSymbolIndex);
			}
		}
		
		template <ParserType THead, ParserType... TTail>
		void PushArguments(zCParser* parser, int startSymbolIndex, const THead& head, const TTail&... tail)
		{
			PushArgument(parser, startSymbolIndex, head);
			startSymbolIndex = startSymbolIndex == -1 ? startSymbolIndex : (startSymbolIndex + 1);
			PushArguments(parser, startSymbolIndex, tail...);
		}

		template <typename T>
			requires ParserType<T> || ParserArgument<T>
		constexpr int GetParserType()
		{
			if constexpr (::Service::Internals::SameAsAny<T, int, bool>)
				return zPAR_TYPE_INT;
			else if constexpr (::Service::Internals::SameAsAny<T, float>)
				return zPAR_TYPE_FLOAT;
			else if constexpr (::Service::Internals::SameAsAny<T, unsigned>)
				return zPAR_TYPE_FUNC;
			else if constexpr (::Service::Internals::SameAsAny<T, zSTRING, const char*>)
				return zPAR_TYPE_STRING;
			else if constexpr (std::is_pointer_v<T>)
				return zPAR_TYPE_INSTANCE;
			else
				return zPAR_TYPE_VOID;
		}

		template <ParserType TReturn, ParserArgument... TArgs>
		void RegisterExternalFunc(zCParser* parser, const char* name, AnyPtr function)
		{
			parser->DefineExternal(name, static_cast<int(*)()>(static_cast<void*>(function)), GetParserType<TReturn>(), GetParserType<TArgs>()..., zPAR_TYPE_VOID);
		}
	}

#pragma endregion

}
