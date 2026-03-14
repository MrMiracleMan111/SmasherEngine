#pragma once
#include "Smasher/Base.h"

namespace Smasher {
	class Engine;
	namespace EngineSystem {
		struct SMASHER_API Context {
			std::reference_wrapper<Engine> engineRef;
		};

		SMASHER_API ErrorCode ClearWindow(entt::registry& registry);
		SMASHER_API ErrorCode DisplayWindow(entt::registry &registry);
	}
}