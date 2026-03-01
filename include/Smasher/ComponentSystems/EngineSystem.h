#pragma once
#include "Smasher/Base.h"

namespace Smasher {
	class Engine;
	namespace EngineSystem {
		struct Context {
			std::reference_wrapper<Engine> engineRef;
		};
	}
}