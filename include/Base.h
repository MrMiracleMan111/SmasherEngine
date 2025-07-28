#pragma once
#include <cstdint>
#include <chrono>
#include "Exceptions.h"
#include "Smasher_export.h"
// EngineAPI.hpp
#pragma once
#define SMASHER_API SMASHERENGINE_EXPORT

namespace Smasher {
	using Millisecond = std::chrono::milliseconds;
	//struct SMASHER_API Millisecond {
	//	double m_Milliseconds;

	//	Millisecond(double milliseconds) : m_Milliseconds(milliseconds) {};

	//	operator double() {
	//		return m_Milliseconds;
	//	}
	//};

	struct Keyboard {
	public:
		enum class KeyboardEventType : int {
			KEY_PRESS,
			KEY_RELEASE,
			KEY_HOLD
		};
	};

	struct Mouse {
	public:
		enum class MouseEventType {
			BUTTON_PRESS,
			BUTTON_RELEASE,
			SCROLL,
			MOUSE_MOVE
		};
	};

	enum class EventType {
		DummyEvent,
		DummyEventExtra,
		KeyboardEvent,
		MouseButtonEvent,
		MouseScrollWheelEvent,
		MouseMoveEvent,
		END
	};
}