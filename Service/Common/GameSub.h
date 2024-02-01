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
		GameSub();

		template <Internals::VoidDelegate Lambda>
		GameSub(const GameEvent& events, Lambda&& lambda);

		template <Internals::VoidDelegate Lambda>
		GameSub(const GameEvent& events, T& option, Lambda&& lambda);

		GameSub(GameSub&& other);
		GameSub& operator=(GameSub&& other);
		~GameSub();

	private:
		void OnOption();

		bool active;
		GameEvent events;
		T* option;
		Delegate<void()> callback;
		Delegate<void()> onOption;
	};

	template <>
	class GameSub<void>
	{
	public:
		GameSub();

		template <Internals::VoidDelegate Lambda>
		GameSub(const GameEvent& events, Lambda&& lambda);

		GameSub(GameSub&& other);
		GameSub& operator=(GameSub&& other);
		~GameSub();

	private:
		GameEvent events;
		Delegate<void()> callback;
	};

	template <typename T>
	GameSub<T>::GameSub() :
		active{},
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

		active = this->events != GameEvent::NoEvent;
		EventManager::GetInstance().Subscribe(this->events, callback);
	}

	template <typename T>
	template <Internals::VoidDelegate Lambda>
	GameSub<T>::GameSub(const GameEvent& events, T& option, Lambda&& lambda) :
		active{},
		events{ events - GameEvent::Execute },
		option{ &option },
		callback{ std::forward<Lambda>(lambda) }
	{
		if (events & GameEvent::Execute)
			if (static_cast<bool>(option))
				callback();

		if (this->events == GameEvent::NoEvent)
			return;

		active = static_cast<bool>(option);
		EventManager::GetInstance().Subscribe(active ? this->events : GameEvent::NoEvent, callback);
		onOption = std::bind(&GameSub::OnOption, this);
		option.OnChange += onOption;
	}

	template <typename T>
	GameSub<T>::GameSub(GameSub&& other) :
		active{ other.active },
		events{ other.events },
		option{ other.option },
		callback{ std::move(other.callback) }
	{
		other.active = {};
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
		if (active)
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

		active = other.active;
		events = other.events;
		option = other.option;
		callback = std::move(other.callback);

		other.active = {};
		other.events = {};
		return *this;
	}

	template <typename T>
	GameSub<T>::~GameSub()
	{
		if (active) EventManager::GetInstance().Unsubscribe(events, callback);
		if (onOption) option->OnChange -= onOption;
	}

	template <typename T>
	void GameSub<T>::OnOption()
	{
		const bool changed = active != static_cast<bool>(*option);
		if (!changed) return;
		active = !active;

		if (active) EventManager::GetInstance().Subscribe(events, callback);
		else EventManager::GetInstance().Unsubscribe(events, callback);
	}

	GameSub<void>::GameSub() :
		events{}
	{

	}

	template <Internals::VoidDelegate Lambda>
	GameSub<void>::GameSub(const GameEvent& events, Lambda&& lambda) :
		events{ events - GameEvent::Execute },
		callback{ std::forward<Lambda>(lambda) }
	{
		if (events & GameEvent::Execute)
			callback();

		if (this->events == GameEvent::NoEvent)
			return;

		EventManager::GetInstance().Subscribe(this->events, callback);
	}

	GameSub<void>::GameSub(GameSub&& other) :
		events{ other.events },
		callback{ std::move(other.callback) }
	{
		other.events = {};
	}

	GameSub<void>& GameSub<void>::operator=(GameSub&& other)
	{
		if (events != GameEvent::NoEvent)
			EventManager::GetInstance().Unsubscribe(events, callback);
		
		events = other.events;
		callback = std::move(other.callback);
		return *this;
	}

	GameSub<void>::~GameSub()
	{
		if (events == GameEvent::NoEvent) return;
		EventManager::GetInstance().Unsubscribe(events, callback);
	}
}
