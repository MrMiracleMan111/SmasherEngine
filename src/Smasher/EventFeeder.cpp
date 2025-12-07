#include "Smasher/EventFeeder.h"
#include "Smasher/EventManager.h"
#include "Smasher/Layer.h"
#include "Smasher/Engine.h"

Smasher::EventFeeder::EventFeeder(EventManager &eventManager, Engine &engine) : m_EventManager(eventManager), m_Engine(engine)
{
	m_KeyboardState.fill(false);
}

void Smasher::EventFeeder::ForwardSFMLEvent(const sf::Event &event)
{
	switch (event.type)
	{
	case sf::Event::Closed:
		m_EventManager.Publish<Events::WindowCloseEvent>();
		break;
	case sf::Event::KeyPressed:
	{
		// case of sf::Key::Unkown
		if (event.key.code < 0) {
			break;
		}
		bool& keyState = m_KeyboardState.at(event.key.code);
		Keyboard::KeyboardEventType keyEventType = keyState ? Keyboard::KeyboardEventType::KEY_HOLD : Keyboard::KeyboardEventType::KEY_PRESS;
		keyState = true;
		m_EventManager.Publish<Events::KeyboardEvent>(keyEventType, event.key.code);
		break;
	}
	case sf::Event::KeyReleased:
		// case of sf::Key::Unkown
		if (event.key.code < 0) {
			break;
		}
		m_KeyboardState.at(event.key.code) = false;
		m_EventManager.Publish<Events::KeyboardEvent>(Keyboard::KeyboardEventType::KEY_RELEASE, event.key.code);
		break;
	case sf::Event::MouseButtonPressed:
		if (!m_Engine.IsHeadless()) {
			sf::Vector2i windowPos = sf::Mouse::getPosition(m_Engine.GetWindow());
			sf::Vector2f localPos = m_Engine.GetWindow().mapPixelToCoords(windowPos);
			m_EventManager.Publish<Events::MouseButtonEvent>(Mouse::MouseEventType::BUTTON_PRESS, event.mouseButton.button, sf::Vector2i(localPos));
		}
		else {
			m_EventManager.Publish<Events::MouseButtonEvent>(Mouse::MouseEventType::BUTTON_PRESS, event.mouseButton.button, sf::Mouse::getPosition());
		}
		break;
	case sf::Event::MouseButtonReleased:
		if (!m_Engine.IsHeadless()) {
			sf::Vector2i windowPos = sf::Mouse::getPosition(m_Engine.GetWindow());
			sf::Vector2f localPos = m_Engine.GetWindow().mapPixelToCoords(windowPos);
			m_EventManager.Publish<Events::MouseButtonEvent>(Mouse::MouseEventType::BUTTON_RELEASE, event.mouseButton.button, sf::Vector2i(localPos));
		}
		else {
			m_EventManager.Publish<Events::MouseButtonEvent>(Mouse::MouseEventType::BUTTON_RELEASE, event.mouseButton.button, sf::Mouse::getPosition());
		}
		break;
	case sf::Event::MouseMoved:
		if (!m_Engine.IsHeadless()) {
			sf::Vector2i windowPos = sf::Mouse::getPosition(m_Engine.GetWindow());
			sf::Vector2f localPos = m_Engine.GetWindow().mapPixelToCoords(windowPos);
			m_EventManager.Publish<Events::MouseMoveEvent>(Mouse::MouseEventType::MOUSE_MOVE, sf::Vector2i(event.mouseMove.x, event.mouseMove.y), sf::Vector2i(localPos));
		}
		else {
			m_EventManager.Publish<Events::MouseMoveEvent>(Mouse::MouseEventType::MOUSE_MOVE, sf::Vector2i(event.mouseMove.x, event.mouseMove.y), sf::Mouse::getPosition());
		}
		break;
	case sf::Event::MouseWheelScrolled:
		if (!m_Engine.IsHeadless()) {
			sf::Vector2i windowPos = sf::Mouse::getPosition(m_Engine.GetWindow());
			sf::Vector2f localPos = m_Engine.GetWindow().mapPixelToCoords(windowPos);
			m_EventManager.Publish<Events::MouseScrollWheelEvent>(Mouse::MouseEventType::SCROLL, event.mouseWheelScroll.delta, sf::Vector2i(localPos));
		}
		else {
			m_EventManager.Publish<Events::MouseScrollWheelEvent>(Mouse::MouseEventType::SCROLL, event.mouseWheelScroll.delta, sf::Mouse::getPosition());
		}
		break;
	case sf::Event::Resized:
		m_EventManager.Publish<Events::WindowResizeEvent>(sf::Vector2u(event.size.width, event.size.height));
		break;
	default:
		break;
	}
}
