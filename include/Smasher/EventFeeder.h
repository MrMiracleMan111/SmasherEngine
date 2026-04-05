#pragma once
#include <array>
#include <SDL3/SDL.h>
#include "Smasher/Base.h"
#include "Smasher/Events.h"
#include "Smasher/EventManager.h"

namespace Smasher {
	class Engine;

	namespace EventFeeder {
		//EventFeeder(EventManager &eventManager, Engine &engine);
		//void ForwardEvent(const sf::Event &event);
		SMASHER_API void ForwardSDLEvent(Engine &engine, const SDL_Event &event);
	};
}
