#pragma once
#include <cstdint>
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
	enum SmasherCapability {
		RENDER = 1 << 0,
		UPDATE = 1 << 1
	};


	struct SMASHER_API Millisecond {
		double m_Milliseconds;

		Millisecond(double milliseconds) : m_Milliseconds(milliseconds) {};

		operator double() {
			return m_Milliseconds;
		}
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