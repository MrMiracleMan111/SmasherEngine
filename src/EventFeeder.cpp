#include "EventFeeder.h"

void Smasher::EventFeeder::ForwardSFMLEvent(const sf::Event& event)
{
	switch (event.type)
	{
	case sf::Event::Closed:
		m_EventManager.Publish<Events::WindowCloseEvent>();
		break;
	case sf::Event::KeyPressed:
		m_EventManager.Publish<Events::KeyboardEvent>(Keyboard::KeyboardEventType::KEY_PRESS, event.key.code);
		break;
	case sf::Event::KeyReleased:
		m_EventManager.Publish<Events::KeyboardEvent>(Keyboard::KeyboardEventType::KEY_RELEASE, event.key.code);
		break;
	case sf::Event::MouseButtonPressed:
		m_EventManager.Publish<Events::MouseButtonEvent>(Mouse::MouseEventType::BUTTON_PRESS, event.mouseButton.button, sf::Mouse::getPosition());
		break;
	case sf::Event::MouseButtonReleased:
		m_EventManager.Publish<Events::MouseButtonEvent>(Mouse::MouseEventType::BUTTON_RELEASE, event.mouseButton.button, sf::Mouse::getPosition());
		break;
	case sf::Event::MouseMoved:
		m_EventManager.Publish<Events::MouseMoveEvent>(Mouse::MouseEventType::MOUSE_MOVE, sf::Vector2i(event.mouseMove.x, event.mouseMove.y), sf::Mouse::getPosition());
		break;
	case sf::Event::MouseWheelScrolled:
		m_EventManager.Publish<Events::MouseScrollWheelEvent>(Mouse::MouseEventType::SCROLL, event.mouseWheelScroll.delta,  sf::Mouse::getPosition());
		break;
	case sf::Event::Resized:
		m_EventManager.Publish<Events::WindowResizeEvent>(sf::Vector2i(event.size.width, event.size.height));
		break;
	default:
		break;
	}
}
