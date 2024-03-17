#pragma once

#include <Service/Common/Delegate.h>

namespace Service
{
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

		Execute = 1u << 21u,
		Destruct = 1u << 22u,

		All = (1u << 23u) - 1u,
		LoadBegin = LoadBegin_NewGame | LoadBegin_SaveGame | LoadBegin_ChangeLevel,
		LoadEnd = LoadEnd_NewGame | LoadEnd_SaveGame | LoadEnd_ChangeLevel
	};

	inline constexpr GameEvent operator+(const GameEvent& x, const GameEvent& y) noexcept;
	inline constexpr GameEvent operator-(const GameEvent& x, const GameEvent& y) noexcept;
	inline constexpr GameEvent operator!(const GameEvent& x) noexcept;
	inline constexpr bool operator&(const GameEvent& x, const GameEvent& y) noexcept;

#pragma push_macro("DEFINE_PROC")
#undef DEFINE_PROC
#define DEFINE_PROC(e) static inline void __fastcall Proc_ ## e() { EventManager::GetInstance().Raise(GameEvent::e); }

	class EventManager
	{
	public:
		inline void Subscribe(const GameEvent& events, const Delegate<void()>& callback);
		inline void Unsubscribe(const GameEvent& events, const Delegate<void()>& callback);
	
		inline void Raise(const GameEvent& singleEvent);
		inline const GameEvent& GetCurrentEvent() const;

		inline static EventManager& GetInstance();

	private:
		class PerFrameCallback
		{
		public:
			inline PerFrameCallback();
			inline virtual void DoWorldPerFrameCallback(class zCWorld* world, class zCCamera* camera);
		};

		inline EventManager();
		EventManager(EventManager&&) = delete;
		EventManager& operator=(EventManager&&) = delete;

		inline void EnsureHookInitialized(uint32_t index);

		static inline uint32_t GetIndex(const GameEvent& event);
		static inline GameEvent GetEvent(const uint32_t index);

		DEFINE_PROC(Entry)

		//DEFINE_PROC(Init)
		static inline void __fastcall Proc_Init()
		{
			EventManager& eventManager = EventManager::GetInstance();
			eventManager.Raise(GameEvent::Init);
			
			if (eventManager.procInitialized[GetIndex(GameEvent::Loop)])
				eventManager.perFrameCallback = std::make_unique<PerFrameCallback>();

			eventManager.initDone = true;
		}

		DEFINE_PROC(Exit)
		DEFINE_PROC(PreLoop)
		DEFINE_PROC(Loop)
		DEFINE_PROC(PostLoop)
		DEFINE_PROC(MenuLoop)
		DEFINE_PROC(SaveBegin)
		DEFINE_PROC(SaveEnd)
		DEFINE_PROC(LoadBegin_NewGame)
		DEFINE_PROC(LoadEnd_NewGame)
		DEFINE_PROC(LoadBegin_SaveGame)
		DEFINE_PROC(LoadEnd_SaveGame)
		DEFINE_PROC(LoadBegin_ChangeLevel)
		DEFINE_PROC(LoadEnd_ChangeLevel)
		DEFINE_PROC(LoadBegin_Trigger)
		DEFINE_PROC(LoadEnd_Trigger)
		DEFINE_PROC(Pause)
		DEFINE_PROC(Unpause)
		DEFINE_PROC(DefineExternals)
		DEFINE_PROC(ApplyOptions)

		static constexpr uint32_t eventsNum = std::countr_zero(static_cast<uint32_t>(GameEvent::All) + 1u) - 2u;
		std::array<DelegateList<void()>, eventsNum> callbacks;
		std::array<::Union::PartialHookNoRegsProc, eventsNum> procs;
		std::array<bool, eventsNum> procInitialized;
		std::unique_ptr<PerFrameCallback> perFrameCallback;
		bool initDone;
		GameEvent currentEvent;
	};

#pragma pop_macro("DEFINE_PROC")
	
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

	constexpr bool operator&(const GameEvent& x, const GameEvent& y) noexcept
	{
		return static_cast<uint32_t>(x) & static_cast<uint32_t>(y);
	}

	void EventManager::Subscribe(const GameEvent& events, const Delegate<void()>& callback)
	{
		uint32_t flags = static_cast<uint32_t>(events);

		for (uint32_t i = 0; flags != 0; i++)
		{
			const uint32_t skip = std::countr_zero(flags);
			i += skip;
			EnsureHookInitialized(i);
			callbacks[i] += callback;
			flags = flags >> (skip + 1);
		}
	}

	void EventManager::Unsubscribe(const GameEvent& events, const Delegate<void()>& callback)
	{
		uint32_t flags = static_cast<uint32_t>(events);

		for (uint32_t i = 0; flags != 0; i++)
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
		static EventManager eventManager;
		return eventManager;
	}

	EventManager::PerFrameCallback::PerFrameCallback()
	{
		auto getVFunc = [](void* object, int funcNumber)
		{
			return *(*reinterpret_cast<void***>(object) + funcNumber);
		};

		class oCGame* ogame = *reinterpret_cast<class oCGame**>(zSwitch(0x008DA6BC, 0x00920D8C, 0x009813DC, 0x00AB0884));
		void** vtable = *reinterpret_cast<void***>(ogame);
		class zCWorld* (__thiscall* GetWorld)(class oCGame*) = reinterpret_cast<decltype(GetWorld)>(getVFunc(ogame, 13));
		class zCWorld* world = GetWorld(ogame);
		void (__thiscall* RegisterPerFrameCallback)(class zCWorld*, class PerFrameCallback*) = reinterpret_cast<decltype(RegisterPerFrameCallback)>(zSwitch(0x005F9CA0, 0x0061B200, 0x00620D60, 0x006284E0));
		RegisterPerFrameCallback(world, this);
	}

	void EventManager::PerFrameCallback::DoWorldPerFrameCallback(class zCWorld* world, class zCCamera* camera)
	{
		EventManager::GetInstance().Raise(GameEvent::Loop);
	}

#pragma push_macro("ADD_PROC")
#undef ADD_PROC
#define ADD_PROC(e) procs[GetIndex(GameEvent::e)] = &EventManager::Proc_ ## e

	EventManager::EventManager() :
		procInitialized{},
		initDone{}
	{
		ADD_PROC(Entry);
		ADD_PROC(Init);
		ADD_PROC(Exit);
		ADD_PROC(PreLoop);
		//ADD_PROC(Loop);
		ADD_PROC(PostLoop);
		ADD_PROC(MenuLoop);
		ADD_PROC(SaveBegin);
		ADD_PROC(SaveEnd);
		ADD_PROC(LoadBegin_NewGame);
		ADD_PROC(LoadEnd_NewGame);
		ADD_PROC(LoadBegin_SaveGame);
		ADD_PROC(LoadEnd_SaveGame);
		ADD_PROC(LoadBegin_ChangeLevel);
		ADD_PROC(LoadEnd_ChangeLevel);
		ADD_PROC(LoadBegin_Trigger);
		ADD_PROC(LoadEnd_Trigger);
		ADD_PROC(Pause);
		ADD_PROC(Unpause);
		ADD_PROC(DefineExternals);
		ADD_PROC(ApplyOptions);

		EnsureHookInitialized(GetIndex(GameEvent::Init));
	}

#pragma pop_macro("ADD_PROC")

	void EventManager::EnsureHookInitialized(uint32_t index)
	{
		if (procInitialized[index])
			return;

		procInitialized[index] = true;
		const GameEvent event = GetEvent(index);

		if (!zSwitch(true, true)) // spacer?
			return;

		if (event == GameEvent::Loop)
		{
			if (initDone)
				perFrameCallback = std::make_unique<PerFrameCallback>();

			return;
		}

		uint32_t (* const zenDef)(uint32_t, uint32_t, uint32_t, uint32_t) = zSwitch<uint32_t>;

		static std::array<uint32_t, eventsNum> addresses =
		{
			zenDef(0x004F3E10 + 8, 0x00506810 + 10, 0x005000F0 + 8, 0x00502D70 + 5), // Entry
			zenDef(0x00637BB6, 0x0065DFFA, 0x00665112, 0x006C1BFF), // Init
			zenDef(0x00424850 + 7, 0x00427310 + 7, 0x004251A0 + 7, 0x004254E0 + 7), // Exit
			zenDef(0x0063DBE0 + 7, 0x006648F0 + 7, 0x0066B930 + 7, 0x006C86A0 + 7), // PreLoop
			0x00000000, // Loop
			zenDef(0x0063DF40, 0x00664CB8, 0x0066BD43, 0x006C8760), // PostLoop
			zenDef(0x004D0E69, 0x004E15A8, 0x004DB349, 0x004DDCF9), // MenuLoop
			zenDef(0x0063AD80 + 8, 0x00661680 + 8, 0x006685D0 + 8, 0x006C5250 + 8), // SaveBegin
			zenDef(0x0063B668, 0x0066208D, 0x00668EC1, 0x006C5B41), // SaveEnd
			zenDef(0x0063C070 + 9, 0x00662B20 + 9, 0x00669970 + 9, 0x006C65A0 + 9), // LoadBegin_NewGame
			zenDef(0x0063C28C, 0x00662D5A, 0x00669B8C, 0x006C67BC), // LoadEnd_NewGame
			zenDef(0x0063C2A0 + 7, 0x00662D60 + 7, 0x00669BA0 + 7, 0x006C67D0 + 7), // LoadBegin_SaveGame
			zenDef(0x0063CD50, 0x0066393C, 0x0066A653, 0x006C7283), // LoadEnd_SaveGame
			zenDef(0x0063CD60 + 7, 0x00663950 + 7, 0x0066A660 + 7, 0x006C7290 + 7), // LoadBegin_ChangeLevel
			zenDef(0x0063D46B, 0x006640F0, 0x0066AD6B, 0x006C7ADD), // LoadEnd_ChangeLevel
			zenDef(0x0063D480 + 10, 0x00664100 + 5, 0x0066AD80 + 10, 0x006C7AF0 + 10), // LoadBegin_Trigger
			zenDef(0x0063D6BE, 0x0066433E, 0x0066AFBE, 0x006C7D2E), // LoadEnd_Trigger
			zenDef(0x0063E1C2, 0x00664F66, 0x0066BFCA, 0x006C8D4A), // Pause
			zenDef(0x0063E2EE, 0x006650A9, 0x0066C0EE, 0x006C8E6E), // Unpause
			zenDef(0x0064E3C0, 0x00676CAF, 0x0067CC6A, 0x006D9B70), // DefineExternals
			zenDef(0x00427993, 0x0042A48F, 0x00428EE5, 0x00429273), // ApplyOptions
		};

		uint32_t address = addresses[index];
		::Union::CreatePartialHook(reinterpret_cast<void*>(address), procs[index]);

		switch (event)
		{
		case GameEvent::Init:
			if (zSwitch(true, true, false, false))
				return;

			address = zenDef(0x00000000, 0x00000000, 0x006650F6, 0x006C1C1B);
			break;

		case GameEvent::PostLoop:
			if (zSwitch(true, true, false, false))
				return;

			address = zenDef(0x00000000, 0x00000000, 0x0066B9F0, 0x006C8AB8);
			break;

		case GameEvent::MenuLoop:
			address = zenDef(0x004D0DF7, 0x004E1536, 0x004DB2D7, 0x004DDC87);
			break;

		default:
			return;
		}

		::Union::CreatePartialHook(reinterpret_cast<void*>(address), procs[index]);
	}

	uint32_t EventManager::GetIndex(const GameEvent& event)
	{
		return std::countr_zero(static_cast<uint32_t>(event));
	}

	GameEvent EventManager::GetEvent(uint32_t index)
	{
		return static_cast<GameEvent>(1u << index);
	}
}
