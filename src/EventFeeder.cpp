#include "EventFeeder.h"

void Smasher::EventFeeder::ForwardSFMLEvent(const sf::Event& event)
{
	switch (event.type)
	{
	case sf::Event::Closed:
		m_EventManager.Publish<Events::WindowCloseEvent>();
		break;
	default:
		break;
	}
}
