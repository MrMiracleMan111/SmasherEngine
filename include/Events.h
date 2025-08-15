#pragma once
#include <chrono>
#include <iostream>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include "Base.h"

#define SMASHER_EVENT_TYPE(type) \
static constexpr EventType GetStaticEventType() { return type; }; \
EventType GetEventType() const override { return GetStaticEventType(); }; \

namespace Smasher {
	using SMASHER_TIMESTAMP = std::chrono::time_point<std::chrono::system_clock>;

	struct SMASHER_API Event {
		SMASHER_TIMESTAMP Timestamp;
		virtual EventType GetEventType() const = 0;
		virtual ~Event() {}
		Event() = delete;

	protected:
		Event(SMASHER_TIMESTAMP timestamp) : Timestamp(timestamp) {}
	};

	namespace Events {	
		struct SMASHER_API DummyEvent : public Event {
			const char *Message;
			SMASHER_EVENT_TYPE(EventType::DummyEvent)
			DummyEvent() = delete;
			DummyEvent(SMASHER_TIMESTAMP timestamp, const char *message) :
				Event(timestamp), Message(message) {
			};
		};

		struct SMASHER_API DummyEventExtra : public Event {
			const char* Message;
			SMASHER_EVENT_TYPE(EventType::DummyEventExtra)
			DummyEventExtra() = delete;
			DummyEventExtra(SMASHER_TIMESTAMP timestamp, const char* message) :
				Event(timestamp), Message(message) {
			};
		};

		struct SMASHER_API KeyboardEvent : public Event {
			Smasher::Keyboard::KeyboardEventType Type;
			sf::Keyboard::Key KeyCode;
			SMASHER_EVENT_TYPE(EventType::KeyboardEvent)
			KeyboardEvent() = delete;
			KeyboardEvent(SMASHER_TIMESTAMP timestamp, Keyboard::KeyboardEventType type, sf::Keyboard::Key keycode) :
				Event(timestamp), Type(type), KeyCode(keycode) {}
		};

		// Intermediate class to represent mouse events
		struct MouseEvent : public Event {
			Mouse::MouseEventType Type;
			virtual EventType GetEventType() const = 0;
			~MouseEvent() {}
		protected:
			MouseEvent() = delete;
			MouseEvent(SMASHER_TIMESTAMP timestamp, Mouse::MouseEventType type) :
				Event(timestamp), Type(type) {}
		};

		struct SMASHER_API MouseButtonEvent : public MouseEvent {
			sf::Mouse::Button ButtonCode;
			sf::Vector2i Position;
			SMASHER_EVENT_TYPE(EventType::MouseButtonEvent)
			MouseButtonEvent(SMASHER_TIMESTAMP timestamp, Mouse::MouseEventType type, sf::Mouse::Button buttoncode, sf::Vector2i position) :
				MouseEvent(timestamp, type), ButtonCode(buttoncode), Position(position) {}
		};

		struct SMASHER_API MouseScrollWheelEvent : public MouseEvent {
			float Delta;
			sf::Vector2i Position;
			SMASHER_EVENT_TYPE(EventType::MouseScrollWheelEvent)
			MouseScrollWheelEvent(SMASHER_TIMESTAMP timestamp, Mouse::MouseEventType type, float delta, sf::Vector2i position) :
				MouseEvent(timestamp, type), Delta(delta), Position(position) {}
		};

		struct SMASHER_API MouseMoveEvent : public MouseEvent {
			sf::Vector2i Delta;
			sf::Vector2i Position;
			SMASHER_EVENT_TYPE(EventType::MouseMoveEvent)
			MouseMoveEvent(SMASHER_TIMESTAMP timestamp, Mouse::MouseEventType type, sf::Vector2i delta, sf::Vector2i position) :
				MouseEvent(timestamp, type), Delta(delta), Position(position) {}
		};

		struct SMASHER_API WindowCloseEvent : public Event {
			SMASHER_EVENT_TYPE(EventType::WindowCloseEvent)
			WindowCloseEvent(SMASHER_TIMESTAMP timestamp) : Event(timestamp) {}
		};
	}
}