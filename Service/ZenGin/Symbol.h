namespace GOTHIC_NAMESPACE
{
	template <typename T>
	concept ParserType = ::Service::Internals::SameAsAny<T, void, int, bool, unsigned, float, zSTRING> || std::is_pointer_v<T>;

	template <typename T>
	concept ParserArgument = ParserType<T> && !::Service::Internals::SameAsAny<T, void> || ::Service::Internals::SameAsAny<T, const char*>;

	enum class SymbolType
	{
		Invalid,
		ClassVar,
		Class,
		ExternalVar,
		ExternalFunc,
		Instance,
		Prototype,
		Func,
		VarInstance,
		VarString,
		VarFloat,
		VarInt
	};

	class Symbol
	{
	public:
		inline Symbol();
		inline Symbol(zCParser* parser, int index);
		inline Symbol(zCParser* parser, const zSTRING& name);
		inline Symbol(const Symbol& right) = default;
		inline Symbol& operator=(const Symbol& right) = default;

		inline explicit operator bool() const;

		inline zCParser* GetParser() const;
		inline int GetIndex() const;
		inline zCPar_Symbol* GetSymbol() const;
		inline SymbolType Classify() const;

		template <ParserType T>
		inline T* AsArray() const;

	private:
		inline void* GetValueAddress() const;

		zCParser* parser;
		int index;
	};

#pragma region Implementation

	Symbol::Symbol() :
		parser{},
		index{}
	{

	}

	Symbol::Symbol(zCParser* parser, int index) :
		parser{ parser },
		index{ index }
	{

	}

	Symbol::Symbol(zCParser* parser, const zSTRING& name) :
		Symbol{ parser, parser->GetIndex(name) }
	{

	}

	Symbol::operator bool() const
	{
		return Classify() != SymbolType::Invalid;
	}

	SymbolType Symbol::Classify() const
	{
		if (!parser)
			return SymbolType::Invalid;

		const zCPar_Symbol* const symbol = parser->GetSymbol(index);

		if (!symbol)
			return SymbolType::Invalid;

		if (symbol->flags & zPAR_FLAG_CLASSVAR)
			return SymbolType::ClassVar;
		else if (symbol->type == zPAR_TYPE_CLASS)
			return SymbolType::Class;
		else if (symbol->flags & zPAR_FLAG_EXTERNAL)
			return symbol->type == zPAR_TYPE_FUNC ? SymbolType::ExternalFunc : SymbolType::ExternalVar;
		else if (symbol->type == zPAR_TYPE_INSTANCE && (symbol->flags & zPAR_FLAG_CONST))
			return SymbolType::Instance;
		else if (symbol->type == zPAR_TYPE_PROTOTYPE)
			return SymbolType::Prototype;
		else if (symbol->type == zPAR_TYPE_FUNC && (symbol->flags & zPAR_FLAG_CONST))
			return SymbolType::Func;
		else if (symbol->type == zPAR_TYPE_INSTANCE)
			return SymbolType::VarInstance;
		else if (symbol->type == zPAR_TYPE_FUNC || symbol->type == zPAR_TYPE_INT)
			return SymbolType::VarInt;
		else if (symbol->type == zPAR_TYPE_FLOAT)
			return SymbolType::VarFloat;
		else if (symbol->type == zPAR_TYPE_STRING)
			return SymbolType::VarString;

		return SymbolType::Invalid;
	}

	zCParser* Symbol::GetParser() const
	{
		return parser;
	}

	zCPar_Symbol* Symbol::GetSymbol() const
	{
		return parser ? parser->GetSymbol(index) : nullptr;
	}

	int Symbol::GetIndex() const
	{
		return index;
	}

	template <ParserType T>
	T* Symbol::AsArray() const
	{
		return static_cast<T*>(GetValueAddress());
	}

	void* Symbol::GetValueAddress() const
	{
		zCPar_Symbol* const symbol = GetSymbol();

		switch (Classify())
		{
		case SymbolType::ClassVar:
			return reinterpret_cast<byte*>(zCPar_Symbol::instance_adr) + symbol->offset;

		case SymbolType::ExternalVar:
			return symbol->adr;

		case SymbolType::ExternalFunc:
		case SymbolType::Instance:
		case SymbolType::Prototype:
		case SymbolType::Func:
			return &symbol->single_intdata;

		case SymbolType::VarInstance:
			return &symbol->offset;

		case SymbolType::VarInt:
		case SymbolType::VarFloat:
			return symbol->ele == 1 ? &symbol->single_intdata : &symbol->intdata[0];

		case SymbolType::VarString:
			return &symbol->stringdata[0];
		}

		return nullptr;
	}

#pragma endregion

}
