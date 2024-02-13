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
		template <ParserType T>
		inline void PopArguments(zCParser* parser, T& argument);

		template <ParserType THead, ParserType... TTail>
		inline void PopArguments(zCParser* parser, THead& head, TTail&... tail);

		inline void PushArguments(zCParser* parser, int junk);

		template <ParserType T>
		inline void PushArguments(zCParser* parser, const T& arg, int useSymbolIndex);

		template <ParserType THead, ParserType... TTail>
		inline void PushArguments(zCParser* parser, const THead& head, const TTail&... tail, int startSymbolIndex);

		template <typename T>
			requires ParserType<T> || ::Service::Internals::SameAsAny<T, void>
		inline constexpr int GetParserType();

		template <typename TReturn, ParserType... TArgs>
			requires ParserType<TReturn> || ::Service::Internals::SameAsAny<TReturn, void>
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
		Internals::PushArguments(parser, value, -1);
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
		Internals::PushArguments<TArgs...>(parser, args..., symbol.GetIndex() + 1);

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
		template <ParserType T>
		void PopArguments(zCParser* parser, T& arg)
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
			PopArguments(parser, head);
		}

		void PushArguments(zCParser* parser, int junk)
		{

		}

		template <ParserType T>
		void PushArguments(zCParser* parser, const T& arg, int useSymbolIndex)
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
		void PushArguments(zCParser* parser, const THead& head, const TTail&... tail, int startSymbolIndex)
		{
			PushArguments(parser, head, startSymbolIndex);
			startSymbolIndex = startSymbolIndex == -1 ? startSymbolIndex : (startSymbolIndex + 1);
			PushArguments(parser, tail..., startSymbolIndex);
		}

		template <typename T>
			requires ParserType<T> || ::Service::Internals::SameAsAny<T, void>
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

		template <typename TReturn, ParserType... TArgs>
			requires ParserType<TReturn> || ::Service::Internals::SameAsAny<TReturn, void>
		void RegisterExternalFunc(zCParser* parser, const char* name, AnyPtr function)
		{
			parser->DefineExternal(name, static_cast<int(*)()>(static_cast<void*>(function)), GetParserType<TReturn>(), GetParserType<TArgs>()..., zPAR_TYPE_VOID);
		}
	}

#pragma endregion

}
