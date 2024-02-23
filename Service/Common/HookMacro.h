#pragma once

#define ENGINE_ADDRESS_OF(function) FindEngineAddress(::Union::SIGNATURE_OF(function), ENGINE)
#define ENGINE_ADDRESS_OF_OVERLOAD(type, function) FindEngineAddress(::Union::Signature(static_cast<type>(function), #function), ENGINE)

#define HOOK_DEFAULT(className, methodName) auto Ivk_ ## className ## _ ## methodName = ::Union::CreateHook( \
	ENGINE_ADDRESS_OF(&className::methodName), \
	&className::methodName ## _Union \
)

#define HOOK_DEFAULT_OVERLOAD(className, methodName, suffix, type) auto Ivk_ ## className ## _ ## methodName ## _ ## suffix = ::Union::CreateHook( \
	ENGINE_ADDRESS_OF_OVERLOAD(type, &className::methodName), \
	static_cast<type>(&className::methodName ## _Union) \
)

#define HOOK_DEFAULT_BINDED(className, methodName, option) BindedHook Ivk_ ## className ## _ ## methodName { \
	ENGINE_ADDRESS_OF(&className::methodName), \
	&className::methodName ## _Union, \
	option \
}

#define HOOK_DEFAULT_BINDED_OVERLOAD(className, methodName, suffix, type, option) BindedHook Ivk_ ## className ## _ ## methodName ## _ ## suffix { \
	ENGINE_ADDRESS_OF_OVERLOAD(type, &className::methodName), \
	static_cast<type>(&className::methodName ## _Union), \
	option \
}

template <typename TFrom, typename TTo>
concept CompatibleHook = requires(TFrom a, TTo b) { b = a; };

#define HOOK(className, methodName, ret, args) \
	struct className ## _ ## methodName : className { ret operator() args; }; \
	static_assert(CompatibleHook<decltype(&className::methodName), decltype(&className ## _ ## methodName :: operator())>, "Incompatible signatures"); \
	BindedHook Ivk_ ## className ## _ ## methodName{ \
		ENGINE_ADDRESS_OF(&className::methodName), &className ## _ ## methodName :: operator() \
	}; \
	ret className ## _ ## methodName :: operator() args

#define HOOK_BINDED(className, methodName, ret, args, option) \
	struct className ## _ ## methodName : className { ret operator() args; }; \
	static_assert(CompatibleHook<decltype(&className::methodName), decltype(&className ## _ ## methodName :: operator())>, "Incompatible signatures"); \
	BindedHook Ivk_ ## className ## _ ## methodName{ \
		ENGINE_ADDRESS_OF(&className::methodName), &className ## _ ## methodName :: operator(), option \
	}; \
	ret className ## _ ## methodName :: operator() args

#define HOOK_OVERLOAD(className, methodName, suffix, ret, args) \
	struct className ## _ ## methodName ## _ ## suffix : className { ret operator() args; }; \
	static_assert(CompatibleHook<ret (className::*) args, decltype(&className ## _ ## methodName ## _ ## suffix :: operator())>, "Incompatible signatures"); \
	BindedHook Ivk_ ## className ## _ ## methodName ## _ ## suffix { \
		ENGINE_ADDRESS_OF_OVERLOAD(ret (className::*) args, &className::methodName), &className ## _ ## methodName ## _ ## suffix :: operator() \
	}; \
	ret className ## _ ## methodName ## _ ## suffix :: operator() args

#define HOOK_BINDED_OVERLOAD(className, methodName, suffix, ret, args, option) \
	struct className ## _ ## methodName ## _ ## suffix : className { ret operator() args; }; \
	static_assert(CompatibleHook<ret (className::*) args, decltype(&className ## _ ## methodName ## _ ## suffix :: operator())>, "Incompatible signatures"); \
	BindedHook Ivk_ ## className ## _ ## methodName ## _ ## suffix { \
		ENGINE_ADDRESS_OF_OVERLOAD(ret (className::*) args, &className::methodName), &className ## _ ## methodName ## _ ## suffix :: operator(), \
		option \
	}; \
	ret className ## _ ## methodName ## _ ## suffix :: operator() args

#define THISCALL(invoker) (this->*invoker)
