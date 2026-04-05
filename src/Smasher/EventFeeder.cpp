#include "Smasher/EventFeeder.h"
#include "Smasher/EventManager.h"
#include "Smasher/Layer.h"
#include "Smasher/Engine.h"

namespace Smasher {
	namespace EventFeeder {
		void ForwardSDLEvent(Engine &engine, const SDL_Event& event)
		{
			EventManager& eventManager = engine.GetEventManager();
			switch (event.type) {
			case SDL_EventType::SDL_EVENT_WINDOW_CLOSE_REQUESTED:
				eventManager.Publish<Events::WindowCloseEvent>();
				break;
			case SDL_EventType::SDL_EVENT_KEY_DOWN:
				// case of sf::Key::Unkown
				if (event.key.key == SDLK_UNKNOWN) {
					break;
				}
				eventManager.Publish<Events::KeyboardEvent>(Keyboard::KeyboardEventType::KEY_PRESS, event.key.key);
				break;
			case SDL_EventType::SDL_EVENT_KEY_UP:
				// case of sf::Key::Unkown
				if (event.key.key == SDLK_UNKNOWN) {
					break;
				}
				eventManager.Publish<Events::KeyboardEvent>(Keyboard::KeyboardEventType::KEY_RELEASE, event.key.key);
				break;
			case SDL_EventType::SDL_EVENT_MOUSE_BUTTON_DOWN:
				if (!engine.IsHeadless()) {
					eventManager.Publish<Events::MouseButtonEvent>(Mouse::MouseEventType::BUTTON_PRESS, event.button.button, glm::vec2{ event.button.x, event.button.y });
				}
				else {
					float x, y;
					SDL_GetGlobalMouseState(&x, &y);
					eventManager.Publish<Events::MouseButtonEvent>(Mouse::MouseEventType::BUTTON_PRESS, event.button.button, glm::vec2{ x, y });
				}
				break;
			case SDL_EventType::SDL_EVENT_MOUSE_BUTTON_UP:
				if (!engine.IsHeadless()) {
					eventManager.Publish<Events::MouseButtonEvent>(Mouse::MouseEventType::BUTTON_RELEASE, event.button.button, glm::vec2{ event.button.x, event.button.y });
				}
				else {
					float x, y;
					SDL_GetGlobalMouseState(&x, &y);
					eventManager.Publish<Events::MouseButtonEvent>(Mouse::MouseEventType::BUTTON_RELEASE, event.button.button, glm::vec2{ x, y });
				}
				break;
			case SDL_EventType::SDL_EVENT_MOUSE_WHEEL:
				if (!engine.IsHeadless()) {
					eventManager.Publish<Events::MouseScrollWheelEvent>(Mouse::MouseEventType::SCROLL, event.wheel.y, glm::vec2{ event.wheel.mouse_x, event.wheel.mouse_y });
				}
				else {
					float x, y;
					SDL_GetGlobalMouseState(&x, &y);
					eventManager.Publish<Events::MouseScrollWheelEvent>(Mouse::MouseEventType::SCROLL, event.wheel.y, glm::vec2{ x, y });
				}
				break;
			case SDL_EventType::SDL_EVENT_MOUSE_MOTION:
				if (!engine.IsHeadless()) {
					eventManager.Publish<Events::MouseMoveEvent>(Mouse::MouseEventType::MOUSE_MOVE, glm::vec2{ event.motion.xrel, event.motion.yrel } , glm::vec2{ event.motion.x, event.motion.y });
				}
				else {
					float x, y, xrel, yrel;
					SDL_GetGlobalMouseState(&x, &y);
					SDL_GetRelativeMouseState(&xrel, &yrel);
					eventManager.Publish<Events::MouseMoveEvent>(Mouse::MouseEventType::MOUSE_MOVE, glm::vec2{ xrel, yrel }, glm::vec2{ x, y });
				}
				break;
			case SDL_EventType::SDL_EVENT_WINDOW_RESIZED:
				eventManager.Publish<Events::WindowResizeEvent>(glm::uvec2{ event.window.data1, event.window.data2 });
				break;
			case SDL_EventType::SDL_EVENT_QUIT:
				break;
			default:
				break;
			}
		}

		void ForwardEvent(const sf::Event& event)
		{
			if (event.is<sf::Event::Closed>()) {
			}
			else if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
				// case of sf::Key::Unkown
				//if ((int)keyPressed->code < 0) {
				//	return;
				//}
				//bool& keyState = m_KeyboardState.at((int)keyPressed->code);
				//Keyboard::KeyboardEventType keyEventType = keyState ? Keyboard::KeyboardEventType::KEY_HOLD : Keyboard::KeyboardEventType::KEY_PRESS;
				//keyState = true;
				//eventManager.Publish<Events::KeyboardEvent>(keyEventType, keyPressed->code);
			}
			else if (const auto* keyReleased = event.getIf<sf::Event::KeyReleased>()) {
				// case of sf::Key::Unkown
				//if ((int)keyReleased->code < 0) {
				//	return;
				//}
				//m_KeyboardState.at((int)keyReleased->code) = false;
				//eventManager.Publish<Events::KeyboardEvent>(Keyboard::KeyboardEventType::KEY_RELEASE, keyReleased->code);
			}
			else if (const auto* buttonPressed = event.getIf<sf::Event::MouseButtonPressed>()) {
				//if (!engine.IsHeadless()) {
				//	sf::Vector2i windowPos = sf::Mouse::getPosition(engine.GetWindow());
				//	sf::Vector2f localPos = engine.GetWindow().mapPixelToCoords(windowPos);
				//	eventManager.Publish<Events::MouseButtonEvent>(Mouse::MouseEventType::BUTTON_PRESS, buttonPressed->button, sf::Vector2i{ localPos });
				//}
				//else {
				//	eventManager.Publish<Events::MouseButtonEvent>(Mouse::MouseEventType::BUTTON_PRESS, buttonPressed->button, sf::Mouse::getPosition());
				//}
			}

			else if (const auto* buttonReleased = event.getIf<sf::Event::MouseButtonReleased>()) {
				//if (!engine.IsHeadless()) {
				//	sf::Vector2i windowPos = sf::Mouse::getPosition(engine.GetWindow());
				//	sf::Vector2f localPos = engine.GetWindow().mapPixelToCoords(windowPos);
				//	eventManager.Publish<Events::MouseButtonEvent>(Mouse::MouseEventType::BUTTON_RELEASE, buttonReleased->button, sf::Vector2i{ localPos });
				//}
				//else {
				//	eventManager.Publish<Events::MouseButtonEvent>(Mouse::MouseEventType::BUTTON_RELEASE, buttonReleased->button, sf::Mouse::getPosition());
				//}
			}
			else if (const auto* mouse = event.getIf<sf::Event::MouseMoved>()) {
				//if (!engine.IsHeadless()) {
				//	sf::Vector2i windowPos = sf::Mouse::getPosition(engine.GetWindow());
				//	sf::Vector2f localPos = engine.GetWindow().mapPixelToCoords(windowPos);
				//	eventManager.Publish<Events::MouseMoveEvent>(Mouse::MouseEventType::MOUSE_MOVE, sf::Vector2i{ mouse->position.x, mouse->position.y }, sf::Vector2i{ localPos });
				//}
				//else {
				//	eventManager.Publish<Events::MouseMoveEvent>(Mouse::MouseEventType::MOUSE_MOVE, sf::Vector2i{ mouse->position.x, mouse->position.y }, sf::Mouse::getPosition());
				//}
			}
			else if (const auto* wheelScrolled = event.getIf<sf::Event::MouseWheelScrolled>()) {
				//if (!engine.IsHeadless()) {
				//	sf::Vector2i windowPos = sf::Mouse::getPosition(engine.GetWindow());
				//	sf::Vector2f localPos = engine.GetWindow().mapPixelToCoords(windowPos);
				//	eventManager.Publish<Events::MouseScrollWheelEvent>(Mouse::MouseEventType::SCROLL, wheelScrolled->delta, sf::Vector2i{ localPos });
				//}
				//else {
				//	eventManager.Publish<Events::MouseScrollWheelEvent>(Mouse::MouseEventType::SCROLL, wheelScrolled->delta, sf::Mouse::getPosition());
				//}
			}
			else if (const auto* size = event.getIf<sf::Event::Resized>()) {
				//eventManager.Publish<Events::WindowResizeEvent>(sf::Vector2u(size->size.x, size->size.y));
			}
			else {
				// Uncaught event
			}
		}
	}
}