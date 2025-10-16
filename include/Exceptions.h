#pragma once
#pragma warning (disable : 4275)
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
		// ==== LAYER EXCEPTIONS ====
		SMASHER_EXCEPTION(LayerNotFound)
		SMASHER_EXCEPTION(LayerDuplicate)
		SMASHER_EXCEPTION(LayerEntityNotFound)
		SMASHER_EXCEPTION(CannotRemoveBaseLayer)

		// ==== COMPONENT EXCEPTIONS ====
		SMASHER_EXCEPTION(ComponentInvalid)
		SMASHER_EXCEPTION(ComponentDowncastFailed)
		SMASHER_EXCEPTION(MissingComponentDependency)

		// ==== CAMERA EXCEPTIONS ====
		SMASHER_EXCEPTION(CameraTargetNotSet)

		// ==== ENTITY EXCEPTIONS ====
		SMASHER_EXCEPTION(EntityDuplicateComponent)
		SMASHER_EXCEPTION(EntityComponentNotFound)

		// ==== EVENT EXCEPTIONS ====
		SMASHER_EXCEPTION(EventHandleInvalid)

		// ==== RESOURCE EXCEPTIONS ====
		SMASHER_EXCEPTION(ResourceFailedToLoad)
		SMASHER_EXCEPTION(ResourceNotLoaded)
		SMASHER_EXCEPTION(ResourceAlreadyExists)
		SMASHER_EXCEPTION(ResourceInvalidNumPaths)

		// ==== OPENGL EXCEPTIONS ====
		SMASHER_EXCEPTION(GLEWInitFailed)
	}
}