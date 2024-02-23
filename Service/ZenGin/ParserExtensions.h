namespace GOTHIC_NAMESPACE
{
#define ZEXTERNAL(name, ret, ...) \
	void name(); \
	namespace Internals \
	{ \
		::Service::GameSub reg_ ## name(ZTEST(GameEvent::DefineExternals), [] \
			{ \
				::GOTHIC_NAMESPACE::Internals::RegisterExternalFunc<ret, __VA_ARGS__>(parser, #name, &name); \
			} \
		); \
	} \
	void name()

	template <ParserType... TArgs>
	inline void zARGS(TArgs&... args);

	template <ParserType T>
	inline void zRETURN(const T& value);

	template <ParserType... TArgs>
	inline void PopArguments(zCParser* parser, TArgs&... args);

	template <ParserType T>
	inline void SetReturn(zCParser* parser, const T& value);

	template <ParserType TReturn, ParserType... TArgs>
	inline TReturn Call(zCParser* parser, const char* function, const TArgs&... args);

	template <ParserType TReturn, ParserType... TArgs>
	inline TReturn Call(zCParser* parser, int function, const TArgs&... args);

	namespace Internals
	{
		inline void PopArguments(zCParser* parser);

		template <ParserType T>
		inline void PopArgument(zCParser* parser, T& argument);

		template <ParserType THead, ParserType... TTail>
		inline void PopArguments(zCParser* parser, THead& head, TTail&... tail);

		inline void PushArguments(zCParser* parser, int junk);

		template <ParserType T>
		inline void PushArgument(zCParser* parser, int useSymbolIndex, const T& arg);

		template <ParserType THead, ParserType... TTail>
		inline void PushArguments(zCParser* parser, int startSymbolIndex, const THead& head, const TTail&... tail);

		template <ParserType T>
		inline constexpr int GetParserType();

		template <ParserType TReturn, ParserType... TArgs>
		inline void RegisterExternalFunc(zCParser* parser, const char* name, AnyPtr function);
	}

#pragma region Implementation

	template <ParserType... TArgs>
	void zARGS(TArgs&... args)
	{
		PopArguments(zCParser::GetParser(), args...);
	}

	template <ParserType T>
	void zRETURN(const T& value)
	{
		SetReturn(zCParser::GetParser(), value);
	}

	template <ParserType... TArgs>
	void PopArguments(zCParser* parser, TArgs&... args)
	{
		Internals::PopArguments(parser, args...);
	}

	template <ParserType T>
	void SetReturn(zCParser* parser, const T& value)
	{
		Internals::PushArguments(parser, -1, value);
	}

	template <ParserType TReturn, ParserType... TArgs>
	TReturn Call(zCParser* parser, const char* function, const TArgs&... args)
	{
		return Call<TReturn>(parser, parser->GetIndex(zSTRING{ function }.Upper()), args...);
	}

	template <ParserType TReturn, ParserType... TArgs>
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

		template <ParserType T>
		void PushArgument(zCParser* parser, int useSymbolIndex, const T& arg)
		{
			if constexpr (::Service::Internals::SameAsAny<T, int, bool, unsigned>)
				parser->SetReturn(static_cast<int>(arg));
			else if constexpr (::Service::Internals::SameAsAny<T, float>)
				parser->SetReturn(arg);
			else if constexpr (::Service::Internals::SameAsAny<T, zSTRING>)
				parser->SetReturn(Internals::StringPool::GetInstance().AddString(arg.ToChar()));
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

		template <ParserType T>
		constexpr int GetParserType()
		{
			if constexpr (::Service::Internals::SameAsAny<T, int, bool>)
				return zPAR_TYPE_INT;
			else if constexpr (::Service::Internals::SameAsAny<T, float>)
				return zPAR_TYPE_FLOAT;
			else if constexpr (::Service::Internals::SameAsAny<T, unsigned>)
				return zPAR_TYPE_FUNC;
			else if constexpr (::Service::Internals::SameAsAny<T, zSTRING>)
				return zPAR_TYPE_STRING;
			else if constexpr (std::is_pointer_v<T>)
				return zPAR_TYPE_INSTANCE;
			else
				return zPAR_TYPE_VOID;
		}

		template <ParserType TReturn, ParserType... TArgs>
		void RegisterExternalFunc(zCParser* parser, const char* name, AnyPtr function)
		{
			parser->DefineExternal(name, static_cast<int(*)()>(static_cast<void*>(function)), GetParserType<TReturn>(), GetParserType<TArgs>()..., zPAR_TYPE_VOID);
		}
	}

#pragma endregion

}
