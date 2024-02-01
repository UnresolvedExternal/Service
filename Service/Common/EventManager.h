#include <Service/Common/Delegate.h>

namespace Service
{
#define LISTEN_GAME_EVENT(name) extern "C" __declspec(dllexport) void Game_ ## name() { ::Service::EventManager::GetInstance().Raise(::Service::GameEvent::name); }

	enum class GameEvent : uint32_t
	{
		NoEvent = 0u,

		Entry = 1u << 0u,
		Init = 1u << 1u,
		Exit = 1u << 2u,
		PreLoop = 1u << 3u,
		Loop = 1u << 4u,
		PostLoop = 1u << 5u,
		MenuLoop = 1u << 6u,
		SaveBegin = 1u << 7u,
		SaveEnd = 1u << 8u,
		LoadBegin_NewGame = 1u << 9u,
		LoadEnd_NewGame = 1u << 10u,
		LoadBegin_SaveGame = 1u << 11u,
		LoadEnd_SaveGame = 1u << 12u,
		LoadBegin_ChangeLevel = 1u << 13u,
		LoadEnd_ChangeLevel = 1u << 14u,
		LoadBegin_Trigger = 1u << 15u,
		LoadEnd_Trigger = 1u << 16u,
		Pause = 1u << 17u,
		Unpause = 1u << 18u,
		DefineExternals = 1u << 19u,
		ApplyOptions = 1u << 20u,
		Detach = 1u << 21u,

		Execute = 1u << 22u,
		All = (1u << 23u) - 1u,
		LoadBegin = LoadBegin_NewGame | LoadBegin_SaveGame | LoadBegin_ChangeLevel,
		LoadEnd = LoadEnd_NewGame | LoadEnd_SaveGame | LoadEnd_ChangeLevel
	};

	constexpr GameEvent operator+(const GameEvent& x, const GameEvent& y) noexcept;
	constexpr GameEvent operator-(const GameEvent& x, const GameEvent& y) noexcept;
	constexpr GameEvent operator!(const GameEvent& x) noexcept;

	class EventManager
	{
	public:
		void Subscribe(const GameEvent& events, const Delegate<void()>& callback);
		void Unsubscribe(const GameEvent& events, const Delegate<void()>& callback);
	
		void Raise(const GameEvent& singleEvent);
		const GameEvent& GetCurrentEvent() const;

		static EventManager& GetInstance();

	private:
		EventManager() = default;
		EventManager(EventManager&&) = delete;
		EventManager& operator=(EventManager&&) = delete;

		static constexpr uint32_t eventsNum = std::countr_zero(static_cast<uint32_t>(GameEvent::All) + 1u) - 1u;
		std::array<DelegateList<void()>, eventsNum> callbacks;
		GameEvent currentEvent;
	};

	constexpr GameEvent operator+(const GameEvent& x, const GameEvent& y) noexcept
	{
		return static_cast<GameEvent>(static_cast<uint32_t>(x) | static_cast<uint32_t>(y));
	}

	constexpr GameEvent operator-(const GameEvent& x, const GameEvent& y) noexcept
	{
		return static_cast<GameEvent>(static_cast<uint32_t>(x) & static_cast<uint32_t>(!y));
	}

	constexpr GameEvent operator!(const GameEvent& x) noexcept
	{
		return static_cast<GameEvent>(~static_cast<uint32_t>(x) & static_cast<uint32_t>(GameEvent::All));
	}

	void EventManager::Subscribe(const GameEvent& events, const Delegate<void()>& callback)
	{
		uint32_t flags = static_cast<uint32_t>(events);

		for (uint32_t i = 0; flags != 0; )
		{
			const uint32_t skip = std::countr_zero(flags);
			i += skip;
			callbacks[i] += callback;
			flags = flags >> (skip + 1);
		}
	}

	void EventManager::Unsubscribe(const GameEvent& events, const Delegate<void()>& callback)
	{
		uint32_t flags = static_cast<uint32_t>(events);

		for (uint32_t i = 0; flags != 0; )
		{
			const uint32_t skip = std::countr_zero(flags);
			i += skip;
			callbacks[i] -= callback;
			flags = flags >> (skip + 1);
		}
	}

	void EventManager::Raise(const GameEvent& singleEvent)
	{
		const uint32_t index = std::countr_zero(static_cast<uint32_t>(singleEvent));
		currentEvent = static_cast<GameEvent>(1 << index);
		callbacks[index]();
		currentEvent = GameEvent::NoEvent;
	}

	const GameEvent& EventManager::GetCurrentEvent() const
	{
		return currentEvent;
	}

	EventManager& EventManager::GetInstance()
	{
		static EventManager eventManager{};
		return eventManager;
	}
}
