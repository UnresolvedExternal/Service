#pragma once
#include <optional>

namespace Service
{
	template <typename TFunction, typename TOption = std::nullptr_t>
	class BindedHook
	{
	public:
		inline BindedHook(::Union::AnyPtr from, TFunction to, ::Union::HookType hookType = ::Union::HookType::Hook_CallPatch);
		inline BindedHook(::Union::AnyPtr from, TFunction to, TOption& option, ::Union::HookType hookType = ::Union::HookType::Hook_CallPatch);
		inline BindedHook(BindedHook&& other);
		inline BindedHook& operator=(BindedHook&& other);
		inline ~BindedHook();

		inline operator TFunction();

	private:
		inline void OnOption();

		void* from;
		TFunction to;
		TOption* option;
		::Union::HookType hookType;
		Delegate<void()> onOption;
		std::optional<::Union::Hook<TFunction>> hook;
		bool enabled;
	};

	template <typename TFunction, typename TOption>
	BindedHook<TFunction, TOption>::BindedHook(::Union::AnyPtr from, TFunction to, ::Union::HookType hookType) :
		from{ *reinterpret_cast<void**>(&from) },
		to{ to },
		option{},
		hookType{ hookType },
		enabled{ from && to }
	{
		if (enabled)
			hook = CreateHook(this->from, to, hookType);
	}

	template <typename TFunction, typename TOption>
	BindedHook<TFunction, TOption>::BindedHook(::Union::AnyPtr from, TFunction to, TOption& option, ::Union::HookType hookType) :
		from{ *reinterpret_cast<void**>(&from) },
		to{ to },
		option{ &option },
		hookType{ hookType },
		enabled{ from && to && static_cast<bool>(option) }
	{
		if (from && to)
		{
			onOption = std::bind(&BindedHook::OnOption, this);
			option.OnChange += onOption;
		}

		if (enabled)
			hook = CreateHook(this->from, to, hookType);
	}

	template <typename TFunction, typename TOption>
	BindedHook<TFunction, TOption>::BindedHook(BindedHook&& other) :
		from{ other.from },
		to{ other.to },
		option{ other.option },
		hookType{ other.hookType },
		hook{ std::move(other.hook) },
		enabled{ other.enabled }
	{
		if constexpr (!std::same_as<TOption, std::nullptr_t>)
			if (other.onOption)
			{
				onOption = std::bind(&BindedHook::OnOption, this);
				option->OnChange.Replace(other.onOption, onOption);
				other.onOption = {};
			}

		other.enabled = false;
	}

	template <typename TFunction, typename TOption>
	BindedHook<TFunction, TOption>& BindedHook<TFunction, TOption>::operator=(BindedHook&& other)
	{
		if (enabled)
			hook->Disable();

		if constexpr (!std::same_as<TOption, std::nullptr_t>)
		{
			if (onOption)
				option->OnChange -= onOption;

			if (other.onOption)
			{
				if (!onOption)
					onOption = std::bind(&BindedHook::OnOption, this);

				option.OnChange().Replace(other.onOption, onOption);
				other.onOption = {};
			}
			else
				onOption = {};
		}

		from = other.from;
		to = other.to;
		option = other.option;
		hookType = other.hookType;
		hook = std::move(other.hook);
		enabled = other.enabled;
		return *this;
	}

	template <typename TFunction, typename TOption>
	BindedHook<TFunction, TOption>::~BindedHook()
	{
		if constexpr (!std::same_as<TOption, std::nullptr_t>)
			if (onOption)
				option->OnChange -= onOption;

		if (enabled)
			hook->Disable();
	}

	template <typename TFunction, typename TOption>
	BindedHook<TFunction, TOption>::operator TFunction()
	{
		if (!hook)
		{
			hook = CreateHook<TFunction>(from, to, hookType);
			hook->Disable();
		}

		return static_cast<TFunction>(*hook);
	}

	template <typename TFunction, typename TOption>
	void BindedHook<TFunction, TOption>::OnOption()
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
