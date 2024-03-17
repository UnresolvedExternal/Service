#include <sstream>

namespace GOTHIC_NAMESPACE
{
#define ZOPTION(name, value) auto name{ ::GOTHIC_NAMESPACE::Internals::CreateActiveOption(PROJECT_NAME, #name, value) }

	class ActiveOptionBase
	{
	public:
		static void LoadAll();

		DelegateList<void()> OnChange;
		std::vector<std::string> StartTrivia;
		std::vector<std::string> EndTrivia;
		bool KeepApartIfHasTrivia;

	protected:
		inline ActiveOptionBase(std::string_view sectionName, std::string_view entryName);
		inline ActiveOptionBase(ActiveOptionBase&) = delete;
		inline ActiveOptionBase& operator=(const ActiveOptionBase&) = delete;
		inline ~ActiveOptionBase();

		inline bool Create();
		inline virtual void Init(bool createdNew) = 0;
		inline virtual void HandleNewValue() = 0;

		zCOptionEntry* entry;
		const std::string sectionName;
		const std::string entryName;

	private:
		inline static int OnOptionChange(zCOptionEntry& e);
		inline static std::string ToUpper(std::string_view text);

		static std::vector<ActiveOptionBase*> activeOptions;
	};

	template <typename T>
	class ActiveOption : public ActiveOptionBase
	{
	public:
		inline ActiveOption(std::string_view sectionName, std::string_view entryName, T&& defaultValue);
		inline ActiveOption(const ActiveOption&) = delete;
		inline ActiveOption& operator=(const ActiveOption&) = delete;

		inline operator T() const;
		inline const T& operator*() const;
		inline const T* operator->() const;
		inline ActiveOption& operator=(const T& newValue);

		template <typename E>
		inline const auto& operator[](const E& index) const;

		const T& GetDefaultValue() const;
		inline void SetDefaultValue(T&& newDefaultValue);

	protected:
		inline virtual void Init(bool createdNew) override;
		inline virtual void HandleNewValue() override;

		OptionValue<T> value;
		OptionValue<T> defaultValue;
	};

	namespace Internals
	{
		template <typename T>
		inline ActiveOption<std::remove_reference_t<T>> CreateActiveOption(const char* section, const char* entry, T&& defaultValue);

		inline ActiveOption<std::string> CreateActiveOption(const char* section, const char* entry, const char* defaultValue);
	}

#pragma region Implementation

	std::vector<ActiveOptionBase*> ActiveOptionBase::activeOptions;

	void ActiveOptionBase::LoadAll()
	{
		for (ActiveOptionBase* option : activeOptions)
		{
			option->Init(option->Create());
			option->entry->ccbList.InsertEnd(OnOptionChange);
		}
	}

	ActiveOptionBase::ActiveOptionBase(std::string_view sectionName, std::string_view entryName) :
		sectionName{ ToUpper(sectionName) },
		entryName{ entryName },
		KeepApartIfHasTrivia{ true }
	{
		activeOptions.push_back(this);
	}

	ActiveOptionBase::~ActiveOptionBase()
	{
		activeOptions.erase(std::find(begin(activeOptions), end(activeOptions), this));
	}

	bool ActiveOptionBase::Create()
	{
		GameSub clearTrivia(GameEvent::Destruct, [this]
		{
			StartTrivia.clear();
			EndTrivia.clear();
		});

		zCOptionSection* const section = zoptions->GetSectionByName(sectionName.c_str(), true);
		entry = zoptions->GetEntryByName(section, entryName.c_str(), false);
		
		if (entry)
			return false;

		const bool hasTrivia = !StartTrivia.empty() || !EndTrivia.empty();
		const bool keepApart = KeepApartIfHasTrivia && hasTrivia;
		const zCOptionEntry* const lastEntry = section->entryList.IsEmpty() ? nullptr : section->entryList[section->entryList.GetNum() - 1];

		if (keepApart)
			if (!lastEntry || !lastEntry->varName.IsEmpty())
				section->entryList.InsertEnd(new zCOptionEntry{ "", "\r\n" });

		for (const std::string& trivia : StartTrivia)
		{
			std::string trash = trivia.empty() ? "\r\n" : ("; " + trivia + "\r\n");
			section->entryList.InsertEnd(new zCOptionEntry{ "", trash.c_str()});
		}

		entry = new zCOptionEntry{ entryName.c_str(), "" };
		section->entryList.InsertEnd(entry);

		for (const std::string& trivia : EndTrivia)
		{
			std::string trash = trivia.empty() ? "\r\n" : ("; " + trivia + "\r\n");
			section->entryList.InsertEnd(new zCOptionEntry{ "", trash.c_str() });
		}

		if (keepApart)
			section->entryList.InsertEnd(new zCOptionEntry{ "", "\r\n" });

		return true;
	}

	int ActiveOptionBase::OnOptionChange(zCOptionEntry& e)
	{
		for (ActiveOptionBase* option : activeOptions)
			if (&e == option->entry)
				option->HandleNewValue();

		return false;
	}

	std::string ActiveOptionBase::ToUpper(std::string_view text)
	{
		std::string result{ text };
		std::transform(begin(result), end(result), begin(result), toupper);
		return result;
	}

	template <typename T>
	ActiveOption<T>::ActiveOption(std::string_view sectionName, std::string_view entryName, T&& defaultValue) :
		ActiveOptionBase{ sectionName, entryName },
		value{},
		defaultValue{ std::forward<T>(defaultValue) }
	{

	}
	
	template <typename T>
	ActiveOption<T>::operator T() const
	{
		return *value;
	}

	template <typename T>
	const T& ActiveOption<T>::operator*() const
	{
		return *value;
	}

	template <typename T>
	const T* ActiveOption<T>::operator->() const
	{
		return &*value;
	}

	template <typename T>
	ActiveOption<T>& ActiveOption<T>::operator=(const T& newValue)
	{
		OptionValue<T> newOptionValue{ newValue };

		if (value == newOptionValue)
			return *this;

		std::ostringstream out;
		out << newOptionValue;
		entry->varValue = entry->varValueTemp = out.str().c_str();

		for (auto* callback : entry->ccbList)
			callback(*entry);

		return *this;
	}

	template <typename T>
	template <typename E>
	const auto& ActiveOption<T>::operator[](const E& index) const
	{
		return value[index];
	}

	template <typename T>
	const T& ActiveOption<T>::GetDefaultValue() const
	{
		return *defaultValue;
	}

	template <typename T>
	void ActiveOption<T>::SetDefaultValue(T&& newDefaultValue)
	{
		defaultValue = std::forward<T>(newDefaultValue);
	}

	template <typename T>
	void ActiveOption<T>::Init(bool createdNew)
	{
		if (createdNew)
		{
			auto setValueFromOverrides = [this]
				{
					if (zgameoptions)
						if (auto* section = zgameoptions->GetSectionByName("OVERRIDES", false))
							if (auto* entry = zgameoptions->GetEntryByName(section, (sectionName + "." + entryName).c_str(), false))
							{
								std::istringstream in{ entry->varValue.ToChar() };
								in >> value;
								return true;
							}

					return false;
				};

			if (!setValueFromOverrides())
				value = defaultValue;

			std::ostringstream out;
			out << value;
			std::string newStringValue{ out.str() };

			entry->varValue = entry->varValueTemp = newStringValue.c_str();
		}
		else
		{
			std::istringstream in{ entry->varValue.ToChar()};
			in >> value;
		}

		OnChange();
	}

	template <typename T>
	void ActiveOption<T>::HandleNewValue()
	{
		OptionValue<T> newValue{};
		std::istringstream in{ entry->varValue.ToChar() };
		in >> newValue;

		if (newValue == value)
			return;

		value = newValue;
		OnChange();
	}

	namespace Internals
	{
		template <typename T>
		ActiveOption<std::remove_reference_t<T>> CreateActiveOption(const char* section, const char* entry, T&& defaultValue)
		{
			return { section, entry, std::move(defaultValue) };
		}

		ActiveOption<std::string> CreateActiveOption(const char* section, const char* entry, const char* defaultValue)
		{
			return { section, entry, std::string{ defaultValue } };
		}
	}

#pragma endregion

}
