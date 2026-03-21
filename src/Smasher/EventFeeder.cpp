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
	if (event.is<sf::Event::Closed>()) {
		m_EventManager.Publish<Events::WindowCloseEvent>();
	}
	else if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>()) {
		// case of sf::Key::Unkown
		if ((int)keyPressed->code < 0) {
			return;
		}
		bool& keyState = m_KeyboardState.at((int)keyPressed->code);
		Keyboard::KeyboardEventType keyEventType = keyState ? Keyboard::KeyboardEventType::KEY_HOLD : Keyboard::KeyboardEventType::KEY_PRESS;
		keyState = true;
		m_EventManager.Publish<Events::KeyboardEvent>(keyEventType, keyPressed->code);
	}
	else if (const auto* keyReleased = event.getIf<sf::Event::KeyReleased>()) {
		// case of sf::Key::Unkown
		if ((int)keyReleased->code < 0) {
			return;
		}
		m_KeyboardState.at((int)keyReleased->code) = false;
		m_EventManager.Publish<Events::KeyboardEvent>(Keyboard::KeyboardEventType::KEY_RELEASE, keyReleased->code);
	}
	else if (const auto* buttonPressed = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (!m_Engine.IsHeadless()) {
			sf::Vector2i windowPos = sf::Mouse::getPosition(m_Engine.GetWindow());
			sf::Vector2f localPos = m_Engine.GetWindow().mapPixelToCoords(windowPos);
			m_EventManager.Publish<Events::MouseButtonEvent>(Mouse::MouseEventType::BUTTON_PRESS, buttonPressed->button, sf::Vector2i{ localPos });
		}
		else {
			m_EventManager.Publish<Events::MouseButtonEvent>(Mouse::MouseEventType::BUTTON_PRESS, buttonPressed->button, sf::Mouse::getPosition());
		}
	}

	else if (const auto* buttonReleased = event.getIf<sf::Event::MouseButtonReleased>()) {
		if (!m_Engine.IsHeadless()) {
			sf::Vector2i windowPos = sf::Mouse::getPosition(m_Engine.GetWindow());
			sf::Vector2f localPos = m_Engine.GetWindow().mapPixelToCoords(windowPos);
			m_EventManager.Publish<Events::MouseButtonEvent>(Mouse::MouseEventType::BUTTON_RELEASE, buttonReleased->button, sf::Vector2i{ localPos });
		}
		else {
			m_EventManager.Publish<Events::MouseButtonEvent>(Mouse::MouseEventType::BUTTON_RELEASE, buttonReleased->button, sf::Mouse::getPosition());
		}
	}
	else if (const auto* mouse = event.getIf<sf::Event::MouseMoved>()) {
		if (!m_Engine.IsHeadless()) {
			sf::Vector2i windowPos = sf::Mouse::getPosition(m_Engine.GetWindow());
			sf::Vector2f localPos = m_Engine.GetWindow().mapPixelToCoords(windowPos);
			m_EventManager.Publish<Events::MouseMoveEvent>(Mouse::MouseEventType::MOUSE_MOVE, sf::Vector2i{ mouse->position.x, mouse->position.y }, sf::Vector2i{ localPos });
		}
		else {
			m_EventManager.Publish<Events::MouseMoveEvent>(Mouse::MouseEventType::MOUSE_MOVE, sf::Vector2i{ mouse->position.x, mouse->position.y }, sf::Mouse::getPosition());
		}
	}
	else if (const auto* wheelScrolled = event.getIf<sf::Event::MouseWheelScrolled>()) {
		if (!m_Engine.IsHeadless()) {
			sf::Vector2i windowPos = sf::Mouse::getPosition(m_Engine.GetWindow());
			sf::Vector2f localPos = m_Engine.GetWindow().mapPixelToCoords(windowPos);
			m_EventManager.Publish<Events::MouseScrollWheelEvent>(Mouse::MouseEventType::SCROLL, wheelScrolled->delta, sf::Vector2i{ localPos });
		}
		else {
			m_EventManager.Publish<Events::MouseScrollWheelEvent>(Mouse::MouseEventType::SCROLL, wheelScrolled->delta, sf::Mouse::getPosition());
		}
	}
	else if (const auto* size = event.getIf<sf::Event::Resized>()) {
		m_EventManager.Publish<Events::WindowResizeEvent>(sf::Vector2u(size->size.x, size->size.y));
	}
	else {
		// Uncaught event
	}
}
