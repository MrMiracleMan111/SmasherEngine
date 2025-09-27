#pragma once
#include <array>
#include "Base.h"
#include "Events.h"
#include "EventManager.h"

namespace Smasher {
	class SMASHER_API EventFeeder {
	public:
		EventFeeder(EventManager& eventManager);
		void ForwardSFMLEvent(const sf::Event& event);
	private:
		EventManager& m_EventManager;
		std::array<bool, sf::Keyboard::KeyCount> m_KeyboardState; // State of keyboard (true means key is pressed)
	};
}
