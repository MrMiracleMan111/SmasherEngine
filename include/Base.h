#pragma once
#include <cstdint>
#include <chrono>
#include "Exceptions.h"
#include "Smasher_export.h"
// EngineAPI.hpp
#pragma once
#define SMASHER_API SMASHERENGINE_EXPORT

#define SMASHER_ADD_CAPABILITIES(args) \
virtual static constexpr uint32_t GetStaticCapabilities() { \
	return (args); \
}


namespace Smasher {
	using Millisecond = std::chrono::milliseconds;
	//struct SMASHER_API Millisecond {
	//	double m_Milliseconds;

	//	Millisecond(double milliseconds) : m_Milliseconds(milliseconds) {};

	//	operator double() {
	//		return m_Milliseconds;
	//	}
	//};

	enum class ComponentStatus {
		VALID,   // Component is active within a manager (not removed)
		INVALID, // Ready to be removed or just created (not assigned to manager yet)
		REMOVED  // Component was removed 
	};

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