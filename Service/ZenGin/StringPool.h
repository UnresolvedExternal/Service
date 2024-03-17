namespace GOTHIC_NAMESPACE
{
	namespace Internals
	{
		class StringPool
		{
		public:
			inline zSTRING& AddString(const char* text);
			inline zSTRING& AddString(const zSTRING& text);

			inline static StringPool& GetInstance();

		private:
			inline StringPool() = default;
			StringPool(StringPool&) = delete;
			StringPool& operator=(const StringPool&) = delete;

			std::vector<std::unique_ptr<zSTRING>> strings;
			GameSub<> cleaner;
		};

#pragma region Implementation

		zSTRING& StringPool::AddString(const char* text)
		{
			if (strings.empty())
				cleaner = { GameEvent::Loop + GameEvent::MenuLoop + GameEvent::Exit, [this]() { strings.clear(); cleaner = {}; } };

			strings.push_back(std::make_unique<zSTRING>(text));
			return *strings.back();
		}

		zSTRING& StringPool::AddString(const zSTRING& text)
		{
			return AddString(text.ToChar());
		}

		StringPool& StringPool::GetInstance()
		{
			static std::unique_ptr<StringPool> instance{ new StringPool{} };
			return *instance;
		}

#pragma endregion

	}
}
