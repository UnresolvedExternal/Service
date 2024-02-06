#pragma once
#include <optional>

namespace Service
{
	template <typename TFunction, typename TOption>
	class ActiveHook
	{
	public:
		template <typename TFrom>
		ActiveHook(TFrom&& from, TFunction to, TOption& option, ::Union::HookType hookType = ::Union::HookType::Hook_Auto);
		
		ActiveHook(ActiveHook&& other);
		ActiveHook& operator=(ActiveHook&& other);
		~ActiveHook();

		operator TFunction();

	private:
		void OnOption();

		void* from;
		TFunction to;
		TOption* option;
		::Union::HookType hookType;
		Delegate<void()> onOption;
		std::optional<::Union::Hook<TFunction>> hook;
		bool enabled;
	};

	template <typename TFunction, typename TOption>
	template <typename TFrom>
	ActiveHook<TFunction, TOption>::ActiveHook(TFrom&& from, TFunction to, TOption& option, ::Union::HookType hookType) :
		from{ *reinterpret_cast<void**>(&from) },
		to{ to },
		option{ &option },
		hookType{ hookType },
		enabled{ from && to && static_cast<bool>(option) }
	{
		if (from && to)
		{
			onOption = std::bind(&ActiveHook::OnOption, this);
			option.OnChange += onOption;
		}

		if (enabled)
			hook = CreateHook(this->from, to, hookType);
	}

	template <typename TFunction, typename TOption>
	ActiveHook<TFunction, TOption>::ActiveHook(ActiveHook&& other) :
		from{ other.from },
		to{ other.to },
		option{ other.option },
		hookType{ other.hookType },
		hook{ std::move(other.hook) },
		enabled{ other.enabled }
	{
		if (!other.onOption)
			return;

		onOption = std::bind(&ActiveHook::OnOption, this);
		option->OnChange.Replace(other.onOption, onOption);
		other.onOption = {};
		other.enabled = false;
	}

	template <typename TFunction, typename TOption>
	ActiveHook<TFunction, TOption>& ActiveHook<TFunction, TOption>::operator=(ActiveHook&& other)
	{
		if (onOption)
			option->OnChange -= onOption;

		if (enabled)
			hook->Disable();

		if (other.onOption)
		{
			if (!onOption)
				onOption = std::bind(&ActiveHook::OnOption, this);

			option.OnChange().Replace(other.onOption, onOption);
			other.onOption = {};
		}
		else
			onOption = {};

		from = other.from;
		to = other.to;
		option = other.option;
		hookType = other.hookType;
		hook = std::move(other.hook);
		enabled = other.enabled;
		return *this;
	}

	template <typename TFunction, typename TOption>
	ActiveHook<TFunction, TOption>::~ActiveHook()
	{
		if (onOption)
			option->OnChange -= onOption;

		if (enabled)
			hook->Disable();
	}

	template <typename TFunction, typename TOption>
	ActiveHook<TFunction, TOption>::operator TFunction()
	{
		if (!hook)
		{
			hook = CreateHook<TFunction>(from, to, hookType);
			hook->Disable();
		}

		return static_cast<TFunction>(*hook);
	}

	template <typename TFunction, typename TOption>
	void ActiveHook<TFunction, TOption>::OnOption()
	{
		if (enabled == static_cast<bool>(*option))
			return;

		enabled = !enabled;

		if (!hook)
			hook = CreateHook<TFunction>(from, to, hookType);
		else if (enabled)
			hook->Enable();
		else
			hook->Disable();
	}
}
