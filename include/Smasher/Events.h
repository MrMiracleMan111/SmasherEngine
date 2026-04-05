#pragma once
#include <chrono>
#include <iostream>
#include <typeindex>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include "Smasher/Base.h"

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
		bool CanPropagate() const { return Propagate; }
		void Block() { Blocked = true; };
		bool IsBlocked() const { return Blocked; }

	protected:
		Event(SMASHER_TIMESTAMP timestamp) : Timestamp(timestamp) {}

	private:
		// Retrieve derived Event type
		virtual std::type_index _GetEventType() const final {
			return std::type_index(typeid(*this));
		}

		bool Propagate = true; // When set to false, event subscriptions will stop handling this event
		bool Blocked = false; // Used to inform subscribers that event was blocked by another subscription, this DOES NOT stop the event from propagating
		// For example, with UI Panels, 
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
			const char *Message;
			DummyEventExtra() = delete;
			DummyEventExtra(SMASHER_TIMESTAMP timestamp, const char *message) :
				Event(timestamp), Message(message) {
			};
		};

		struct SMASHER_API KeyboardEvent : public Event {
			Smasher::Keyboard::KeyboardEventType Type;
			SDL_Keycode KeyCode;
			KeyboardEvent() = delete;
			KeyboardEvent(SMASHER_TIMESTAMP timestamp, Keyboard::KeyboardEventType type, SDL_Keycode keycode) :
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
			Uint8 ButtonCode;
			glm::vec2 Position;
			MouseButtonEvent(SMASHER_TIMESTAMP timestamp, Mouse::MouseEventType type, Uint8 buttoncode, glm::vec2 position) :
				MouseEvent(timestamp, type), ButtonCode(buttoncode), Position(position) {}
		};

		struct SMASHER_API MouseScrollWheelEvent : public MouseEvent {
			float Delta;
			glm::vec2 Position;
			MouseScrollWheelEvent(SMASHER_TIMESTAMP timestamp, Mouse::MouseEventType type, float delta, glm::vec2 position) :
				MouseEvent(timestamp, type), Delta(delta), Position(position) {}
		};

		struct SMASHER_API MouseMoveEvent : public MouseEvent {
			glm::vec2 Delta;
			glm::vec2 Position;
			MouseMoveEvent(SMASHER_TIMESTAMP timestamp, Mouse::MouseEventType type, glm::vec2 delta, glm::vec2 position) :
				MouseEvent(timestamp, type), Delta(delta), Position(position) {}
		};

		struct SMASHER_API WindowCloseEvent : public Event {
			WindowCloseEvent(SMASHER_TIMESTAMP timestamp) : Event(timestamp) {}
		};

		struct SMASHER_API WindowResizeEvent : public Event {
			glm::uvec2 WindowSize;
				WindowResizeEvent(SMASHER_TIMESTAMP timestamp, glm::uvec2 size) :
				Event(timestamp), WindowSize(size) {}
		};
	}
}