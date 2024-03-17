#pragma once

#include <Service/Common/EventManager.h>

namespace Service
{
	namespace Internals
	{
		template <typename T>
		concept VoidDelegate = requires(T & delegate)
		{
			{ delegate() } -> std::same_as<void>;
		};
	}

	// EventManager subscription with option
	template <typename T = void>
	class GameSub
	{
	public:
		inline GameSub();

		template <Internals::VoidDelegate Lambda>
		inline GameSub(const GameEvent& events, Lambda&& lambda);

		template <Internals::VoidDelegate Lambda>
		inline GameSub(const GameEvent& events, T& option, Lambda&& lambda);

		inline GameSub(GameSub&& other);
		inline GameSub& operator=(GameSub&& other);
		inline ~GameSub();

	private:
		inline void OnOption();

		bool subscribed;
		GameEvent events;
		T* option;
		Delegate<void()> callback;
		Delegate<void()> onOption;
	};

	template <>
	class GameSub<void>
	{
	public:
		inline GameSub() noexcept;

		template <Internals::VoidDelegate Lambda>
		inline GameSub(const GameEvent& events, Lambda&& lambda) noexcept;

		inline GameSub(GameSub&& other) noexcept;
		inline GameSub& operator=(GameSub&& other) noexcept;
		inline ~GameSub() noexcept;

	private:
		GameEvent events;
		Delegate<void()> callback;
	};

	template <typename T>
	GameSub<T>::GameSub() :
		subscribed{},
		events{},
		option{}
	{

	}

	template <typename T>
	template <Internals::VoidDelegate Lambda>
	GameSub<T>::GameSub(const GameEvent& events, Lambda&& lambda) :
		events{ events - GameEvent::Execute },
		option{},
		callback{ std::forward<Lambda>(lambda) }
	{
		if (events & GameEvent::Execute)
			callback();

		subscribed = this->events - GameEvent::Destruct != GameEvent::NoEvent;
		EventManager::GetInstance().Subscribe(this->events - GameEvent::Destruct, callback);
	}

	template <typename T>
	template <Internals::VoidDelegate Lambda>
	GameSub<T>::GameSub(const GameEvent& events, T& option, Lambda&& lambda) :
		subscribed{},
		events{ events - GameEvent::Execute },
		option{ &option },
		callback{ std::forward<Lambda>(lambda) }
	{
		if (events & GameEvent::Execute)
			if (static_cast<bool>(option))
				callback();

		if (this->events - GameEvent::Destruct == GameEvent::NoEvent)
			return;

		subscribed = static_cast<bool>(option);
		EventManager::GetInstance().Subscribe(subscribed ? this->events : GameEvent::NoEvent, callback);
		onOption = std::bind(&GameSub::OnOption, this);
		option.OnChange += onOption;
	}

	template <typename T>
	GameSub<T>::GameSub(GameSub&& other) :
		subscribed{ other.subscribed },
		events{ other.events },
		option{ other.option },
		callback{ std::move(other.callback) }
	{
		other.subscribed = {};
		other.events = {};

		if (!other.onOption)
			return;

		onOption = std::bind(&GameSub::OnOption, this);
		option->OnChange.Replace(other.onOption, onOption);
		other.onOption = {};
	}

	template <typename T>
	GameSub<T>& GameSub<T>::operator=(GameSub<T>&& other)
	{
		if (subscribed)
			EventManager::GetInstance().Unsubscribe(events, callback);

		if (onOption)
		{
			option->OnChange -= onOption;
			if (!other.onOption) onOption = {};
		}
		else if (other.onOption)
		{
			onOption = std::bind(&GameSub::OnOption, this);
			other.option->OnChange.Replace(other.onOption, onOption);
			other.onOption = {};
		}

		subscribed = other.subscribed;
		events = other.events;
		option = other.option;
		callback = std::move(other.callback);

		other.subscribed = {};
		other.events = {};
		return *this;
	}

	template <typename T>
	GameSub<T>::~GameSub()
	{
		if (subscribed) EventManager::GetInstance().Unsubscribe(events, callback);
		if (onOption) option->OnChange -= onOption;
		
		if (events & GameEvent::Destruct)
			if (!option || static_cast<bool>(*option))
				callback();
	}

	template <typename T>
	void GameSub<T>::OnOption()
	{
		const bool changed = subscribed != static_cast<bool>(*option);
		if (!changed) return;
		subscribed = !subscribed;

		if (subscribed) EventManager::GetInstance().Subscribe(events, callback);
		else EventManager::GetInstance().Unsubscribe(events, callback);
	}

	GameSub<void>::GameSub() noexcept :
		events{}
	{

	}

	template <Internals::VoidDelegate Lambda>
	GameSub<void>::GameSub(const GameEvent& events, Lambda&& lambda) noexcept :
		events{ events - GameEvent::Execute },
		callback{ std::forward<Lambda>(lambda) }
	{
		if (events & GameEvent::Execute)
			callback();

		if (this->events - GameEvent::Destruct == GameEvent::NoEvent)
			return;

		EventManager::GetInstance().Subscribe(this->events - GameEvent::Destruct, callback);
	}

	GameSub<void>::GameSub(GameSub&& other) noexcept :
		events{ other.events },
		callback{ std::move(other.callback) }
	{
		other.events = {};
	}

	GameSub<void>& GameSub<void>::operator=(GameSub&& other) noexcept
	{
		if (events - GameEvent::Destruct != GameEvent::NoEvent)
			EventManager::GetInstance().Unsubscribe(events - GameEvent::Destruct, callback);
		
		events = other.events;
		callback = std::move(other.callback);
		other.events = {};
		return *this;
	}

	GameSub<void>::~GameSub() noexcept
	{
		if (events - GameEvent::Destruct != GameEvent::NoEvent)
			EventManager::GetInstance().Unsubscribe(events - GameEvent::Destruct, callback);

		if (events & GameEvent::Destruct)
			callback();
	}
}
