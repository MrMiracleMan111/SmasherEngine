#pragma once
#include <exception>
#include <stdexcept>
#include "Smasher_export.h"
#define SMASHER_API SMASHERENGINE_EXPORT

#define SMASHER_EXCEPTION(name) class SMASHER_API name : public std::exception { \
public: \
explicit name() : m_Message("") {} \
explicit name(const std::string& msg) : m_Message(msg) {} \
const char *what() const noexcept {return m_Message.c_str();} \
private: \
const std::string& m_Message; \
}; \

namespace Smasher {
	namespace Exceptions {
		// ==== GAME STATE EXCEPTIONS ====
		SMASHER_EXCEPTION(GameStateNotFound)
		SMASHER_EXCEPTION(GameStateDuplicate)
		SMASHER_EXCEPTION(GameStateEntityNotFound)

		// ==== COMPONENT EXCEPTIONS ====
		SMASHER_EXCEPTION(ComponentInvalid)
		SMASHER_EXCEPTION(ComponentDowncastFailed)

		// ==== ENTITY EXCEPTIONS ====
		SMASHER_EXCEPTION(EntityDuplicateComponent)
		SMASHER_EXCEPTION(EntityComponentNotFound)

		// ==== EVENT EXCEPTIONS ====
		SMASHER_EXCEPTION(EventHandleInvalid)

		// ==== RESOURCE EXCEPTIONS ====
		SMASHER_EXCEPTION(ResourceFailedToLoad)
		SMASHER_EXCEPTION(ResourceNotLoaded)
		SMASHER_EXCEPTION(ResourceAlreadyExists)
		SMASHER_EXCEPTION(ResourceTypeMismatch)
	}
}