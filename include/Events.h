#pragma once
#include <chrono>
#include <iostream>
#include <typeindex>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include "Base.h"

namespace Smasher {
	using SMASHER_TIMESTAMP = std::chrono::time_point<std::chrono::system_clock>;

	struct SMASHER_API Event {
		friend class EventManager;
		friend class Layer;
		SMASHER_TIMESTAMP Timestamp;
		std::type_index GetEventType() const { return _GetEventType(); }

		virtual ~Event() {}
		Event() = delete;

		void StopPropagate() { Propagate = false; };

	protected:
		Event(SMASHER_TIMESTAMP timestamp) : Timestamp(timestamp) {}
		bool Propagate = true; // When set to false, event subscriptions will stop handling this event
	
	private:
		// Retrieve derived Event type
		virtual std::type_index _GetEventType() const final {
			return std::type_index(typeid(*this));
		}
	};

	namespace Events {	
		struct SMASHER_API DummyEvent : public Event {
			const char *Message;
			DummyEvent() = delete;
			DummyEvent(SMASHER_TIMESTAMP timestamp, const char *message) :
				Event(timestamp), Message(message) {
			};
		};

		struct SMASHER_API DummyEventExtra : public Event {
			const char* Message;
			DummyEventExtra() = delete;
			DummyEventExtra(SMASHER_TIMESTAMP timestamp, const char* message) :
				Event(timestamp), Message(message) {
			};
		};

		struct SMASHER_API KeyboardEvent : public Event {
			Smasher::Keyboard::KeyboardEventType Type;
			sf::Keyboard::Key KeyCode;
			KeyboardEvent() = delete;
			KeyboardEvent(SMASHER_TIMESTAMP timestamp, Keyboard::KeyboardEventType type, sf::Keyboard::Key keycode) :
				Event(timestamp), Type(type), KeyCode(keycode) {}
		};

		// Intermediate class to represent mouse events
		struct MouseEvent : public Event {
			Mouse::MouseEventType Type;
			~MouseEvent() {}
		protected:
			MouseEvent() = delete;
			MouseEvent(SMASHER_TIMESTAMP timestamp, Mouse::MouseEventType type) :
				Event(timestamp), Type(type) {}
		};

		struct SMASHER_API MouseButtonEvent : public MouseEvent {
			sf::Mouse::Button ButtonCode;
			sf::Vector2i Position;
			MouseButtonEvent(SMASHER_TIMESTAMP timestamp, Mouse::MouseEventType type, sf::Mouse::Button buttoncode, sf::Vector2i position) :
				MouseEvent(timestamp, type), ButtonCode(buttoncode), Position(position) {}
		};

		struct SMASHER_API MouseScrollWheelEvent : public MouseEvent {
			float Delta;
			sf::Vector2i Position;
			MouseScrollWheelEvent(SMASHER_TIMESTAMP timestamp, Mouse::MouseEventType type, float delta, sf::Vector2i position) :
				MouseEvent(timestamp, type), Delta(delta), Position(position) {}
		};

		struct SMASHER_API MouseMoveEvent : public MouseEvent {
			sf::Vector2i Delta;
			sf::Vector2i Position;
			MouseMoveEvent(SMASHER_TIMESTAMP timestamp, Mouse::MouseEventType type, sf::Vector2i delta, sf::Vector2i position) :
				MouseEvent(timestamp, type), Delta(delta), Position(position) {}
		};

		struct SMASHER_API WindowCloseEvent : public Event {
			WindowCloseEvent(SMASHER_TIMESTAMP timestamp) : Event(timestamp) {}
		};

		struct SMASHER_API WindowResizeEvent : public Event {
			sf::Vector2u WindowSize;
				WindowResizeEvent(SMASHER_TIMESTAMP timestamp, sf::Vector2u size) :
				Event(timestamp), WindowSize(size) {}
		};
	}
}