#pragma once
#include "Base.h"
#include "Events.h"
#include "EventManager.h"

namespace Smasher {
	class SMASHER_API EventFeeder {
	public:
		EventFeeder(EventManager& eventManager) : m_EventManager(eventManager) {}
		void ForwardSFMLEvent(const sf::Event& event);
	private:
		EventManager& m_EventManager;
	};
}
