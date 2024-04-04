namespace GOTHIC_NAMESPACE
{
	enum class KeyToggling
	{
		None,
		NonModifiers,
		All
	};

	class PrimitiveKeyCombo
	{
	public:
		PrimitiveKeyCombo() = default;
		inline PrimitiveKeyCombo(std::string_view text);
		PrimitiveKeyCombo(const PrimitiveKeyCombo&) = default;
		PrimitiveKeyCombo(PrimitiveKeyCombo&&) = default;
		PrimitiveKeyCombo& operator=(const PrimitiveKeyCombo&) = default;
		PrimitiveKeyCombo& operator=(PrimitiveKeyCombo&&) = default;

		inline bool operator==(const PrimitiveKeyCombo& other) const;

		inline void Clear();
		inline explicit operator bool() const;

		template <typename T>
		inline void AddChoice(const T& keys);

		inline void SetToPressedKeys();

		inline bool IsPressed() const;
		inline bool IsToggled(KeyToggling toggling = KeyToggling::NonModifiers) const;

		static inline bool IsModifier(int key);
		static inline constexpr int GetMaxKeys();
		static inline bool* GetKeyEvent();
		static inline bool* GetKeyToggle();

		friend inline std::istream& operator>>(std::istream& in, PrimitiveKeyCombo& combo);
		friend inline std::ostream& operator<<(std::ostream& out, const PrimitiveKeyCombo& combo);

	private:
		std::vector<std::vector<int>> choices;
	};

	class KeyCombo
	{
	public:
		KeyCombo() = default;
		inline KeyCombo(std::string_view text);
		KeyCombo(const KeyCombo&) = default;
		KeyCombo(KeyCombo&&) = default;
		KeyCombo& operator=(const KeyCombo&) = default;
		KeyCombo& operator=(KeyCombo&&) = default;

		inline bool operator==(const KeyCombo& other) const;

		inline void Clear();
		inline explicit operator bool() const;
		inline bool IsPressed() const;
		inline bool IsToggled(KeyToggling toggling = KeyToggling::NonModifiers) const;

		friend inline std::istream& operator>>(std::istream& in, KeyCombo& combo);
		friend inline std::ostream& operator<<(std::ostream& out, const KeyCombo& combo);

	private:
		std::vector<PrimitiveKeyCombo> combos;
	};

#pragma region Implementation

	PrimitiveKeyCombo::PrimitiveKeyCombo(std::string_view text)
	{
		std::istringstream in{ std::string{ text } };
		in >> *this;
	}

	bool PrimitiveKeyCombo::operator==(const PrimitiveKeyCombo& other) const
	{
		if (choices.size() != other.choices.size())
			return false;

		for (size_t i = 0; i < choices.size(); i++)
		{
			if (choices[i].size() != other.choices[i].size())
				return false;

			for (size_t k = 0; k < choices[i].size(); k++)
				if (choices[i][k] != other.choices[i][k])
					return false;
		}

		return true;
	}

	void PrimitiveKeyCombo::Clear()
	{
		choices.clear();
	}

	PrimitiveKeyCombo::operator bool() const
	{
		return !choices.empty();
	}

	template <typename T>
	void PrimitiveKeyCombo::AddChoice(const T& keys)
	{
		std::vector<int>& choice = choices.emplace_back(keys.size());
		auto it = begin(keys);

		for (size_t i = 0; i < keys.size(); i++, it++)
			choice[i] = *it;
	}

	void PrimitiveKeyCombo::SetToPressedKeys()
	{
		choices.clear();

		for (int i = 1; i < GetMaxKeys(); i++)
			if (zinput->KeyPressed(i))
				choices.emplace_back(1)[0] = i;
	}

	bool PrimitiveKeyCombo::IsPressed() const
	{
		for (const std::vector<int>& choice : choices)
		{
			bool pressed = false;

			for (int key : choice)
			{
				if (key >= GetMaxKeys())
					continue;

				if (::Service::Internals::KeyNameHelper::GetInstance().GetName(key).empty())
					continue;

				pressed = zinput->KeyPressed(key);

				if (pressed)
					break;
			}

			if (!pressed)
				return false;
		}

		return true;
	}

	bool PrimitiveKeyCombo::IsToggled(KeyToggling toggling) const
	{
		std::vector<int> keys;

		for (const std::vector<int>& choice : choices)
		{
			bool toggled = false;

			for (int key : choice)
			{
				if (key >= GetMaxKeys())
					continue;

				if (::Service::Internals::KeyNameHelper::GetInstance().GetName(key).empty())
					continue;

				toggled = IsModifier(key) ? zinput->KeyPressed(key) : GetKeyToggle()[key];

				if (toggled)
				{
					keys.push_back(key);
					break;
				}
			}

			if (!toggled)
				return false;
		}

		if (toggling == KeyToggling::None)
			return true;

		for (int key : keys)
			if (toggling != KeyToggling::NonModifiers || !IsModifier(key))
				zinput->KeyToggled(key);

		return true;
	}

	bool PrimitiveKeyCombo::IsModifier(int key)
	{
		for (int modifier : { KEY_LALT, KEY_RALT, KEY_LSHIFT, KEY_RSHIFT, KEY_LCONTROL, KEY_RCONTROL, KEY_LWIN, KEY_RWIN })
			if (modifier == key)
				return true;

		return false;
	}

	constexpr int PrimitiveKeyCombo::GetMaxKeys()
	{
		return ZENDEF(512, 532, 576, 576);
	}

	bool* PrimitiveKeyCombo::GetKeyEvent()
	{
		return reinterpret_cast<bool*>(ZENDEF(0x0086CCC8, 0x008B27C0, 0x008C3020, 0x008D1678));
	}

	bool* PrimitiveKeyCombo::GetKeyToggle()
	{
		return reinterpret_cast<bool*>(ZENDEF(0x0086CED0, 0x008B29D4, 0x008C3260, 0x008D18B8));
	}

	inline std::istream& operator>>(std::istream& in, PrimitiveKeyCombo& combo)
	{
		combo.Clear();
		std::vector<int> choice;

		GameSub<void> addLastChoice{ GameEvent::Destruct, [&]()
		{
			if (!choice.empty())
				combo.AddChoice(choice);
		}};

		auto getWord = [&]()
		{
			std::string word;

			while (in && isspace(in.peek()))
				in.get();

			if (!in)
				return word;

			word += static_cast<char>(in.get());

			if (word == "+" || word == "*")
				return word;

			while (in.peek() == '_' || isalpha(in.peek()) || isdigit(in.peek()))
				word += static_cast<char>(in.get());

			return word;
		};

		while (true)
		{
			std::string word = getWord();

			if (word.empty())
				return in;

			if (word == "+")
			{
				if (!choice.empty())
					combo.AddChoice(choice);

				choice.clear();
				continue;
			}

			if (word == "*")
				continue;

			const int key = ::Service::Internals::KeyNameHelper::GetInstance().GetCode(word);

			if (key == 0)
			{
				addLastChoice = {};
				combo.Clear();
				return in;
			}

			choice.push_back(key);
		}
	}

	inline std::ostream& operator<<(std::ostream& out, const PrimitiveKeyCombo& combo)
	{
		if (combo.choices.empty())
			return out << "KEY_NONE";

		bool anything = false;

		for (const std::vector<int>& choice : combo.choices)
		{
			bool first = true;

			for (size_t i = 0; i < choice.size(); i++)
			{
				std::string_view name = ::Service::Internals::KeyNameHelper::GetInstance().GetName(choice[i]);

				if (name.empty())
					continue;

				if (first && anything)
					out << " + ";

				if (!first)
					out << " * ";

				out << name;
				first = false;
			}

			anything = anything || !first;
		}

		if (!anything)
			return out << "KEY_NONE";

		return out;
	}

	KeyCombo::KeyCombo(std::string_view text)
	{
		std::istringstream in{ std::string{ text } };
		in >> *this;
	}

	bool KeyCombo::operator==(const KeyCombo& other) const
	{
		if (combos.size() != other.combos.size())
			return false;

		for (size_t i = 0; i < combos.size(); i++)
			if (combos[i] != other.combos[i])
				return false;

		return true;
	}

	void KeyCombo::Clear()
	{
		combos.clear();
	}

	KeyCombo::operator bool() const
	{
		return !combos.empty();
	}

	bool KeyCombo::IsPressed() const
	{
		for (const PrimitiveKeyCombo& combo : combos)
			if (combo.IsPressed())
				return true;

		return false;
	}

	bool KeyCombo::IsToggled(KeyToggling toggling) const
	{
		for (const PrimitiveKeyCombo& combo : combos)
			if (combo.IsToggled(toggling))
				return true;

		return false;
	}

	std::istream& operator>>(std::istream& in, KeyCombo& combo)
	{
		combo.Clear();

		std::string text;
		std::getline(in, text);
		text.append(",");
		
		size_t start = 0;

		while (start < text.size())
		{
			size_t end = text.find(",", start);

			if (end == std::string::npos)
				break;

			PrimitiveKeyCombo& primitiveCombo = combo.combos.emplace_back(std::string_view{ text.begin() + start, text.begin() + end });
			
			if (!static_cast<bool>(primitiveCombo))
			{
				combo.Clear();
				return in;
			}

			start = end + 1;
		}

		return in;
	}

	std::ostream& operator<<(std::ostream& out, const KeyCombo& combo)
	{
		if (!static_cast<bool>(combo))
			return out << "KEY_NONE";

		for (size_t i = 0; i < combo.combos.size(); i++)
		{
			if (i != 0)
				out << ", ";

			out << combo.combos[i];
		}

		return out;
	}

#pragma endregion

}