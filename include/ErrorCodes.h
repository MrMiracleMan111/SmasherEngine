#include "Smasher/Base.h"

// ==== LAYER EXCEPTIONS ====
#define ERROR_LayerNotFound 100
#define ERROR_LayerDuplicate 101
#define ERROR_LayerEntityNotFound 102
#define ERROR_CannotRemoveBaseLayer 103

// ==== COMPONENT EXCEPTIONS ====
#define ERROR_ComponentInvalid 200
#define ERROR_ComponentDowncastFailed 202
#define ERROR_MissingComponentDependency 203

// ==== CAMERA EXCEPTIONS ====
#define ERROR_CameraTargetNotSet 300

// ==== ENTITY EXCEPTIONS ====
#define ERROR_EntityDuplicateComponent 400
#define ERROR_EntityComponentNotFound 401

// ==== EVENT EXCEPTIONS ====
#define ERROR_EventHandleInvalid 500

// ==== RESOURCE EXCEPTIONS ====
#define ERROR_ResourceFailedToLoad 600
#define ERROR_ResourceNotLoaded 601
#define ERROR_ResourceAlreadyExists 602
#define ERROR_ResourceInvalidNumPaths 603

// ==== PHYSICS EXCEPTIONS ====
#define ERROR_Box2DWorldAlreadyCreated 700
#define ERROR_Box2DBodyIdInvalid 701

// ==== OPENGL EXCEPTIONS ====
#define ERROR_GLEWInitFailed 800


namespace Smasher {
	const char* GetErrorMessage(ErrorCode code) {
		switch (code) {
				// ==== LAYER EXCEPTIONS ====
			case ERROR_LayerNotFound: return "LayerNotFound";
			case ERROR_LayerDuplicate: return "LayerDuplicate";
			case ERROR_LayerEntityNotFound: return "LayerEntityNotFound";
			case ERROR_CannotRemoveBaseLayer: return "CannotRemoveBaseLayer";

				// ==== COMPONENT EXCEPTIONS ====
			case ERROR_ComponentInvalid: return "Component invalid";
			case ERROR_ComponentDowncastFailed: return "Component downcast failed";
			case ERROR_MissingComponentDependency: return "Missing component dependency";

				// ==== CAMERA EXCEPTIONS ====
			case ERROR_CameraTargetNotSet: return "Camera target not set";

				// ==== ENTITY EXCEPTIONS ====
			case ERROR_EntityDuplicateComponent: return "Attempted to add duplicate component (an entity's components cannot share the same type)";
			case ERROR_EntityComponentNotFound: return "Could not find component";

				// ==== EVENT EXCEPTIONS ====
			case ERROR_EventHandleInvalid: return "EventHandle is invalid";

				// ==== RESOURCE EXCEPTIONS ====
			case ERROR_ResourceFailedToLoad: return "Resource failed to load";
			case ERROR_ResourceNotLoaded: return "Resource is not loaded";
			case ERROR_ResourceAlreadyExists: return "Resource with ID already exists";
			case ERROR_ResourceInvalidNumPaths: return "Resource has an invalid number of paths";

				// ==== PHYSICS EXCEPTIONS ====
			case ERROR_Box2DWorldAlreadyCreated: return "Box2D world was already created";
			case ERROR_Box2DBodyIdInvalid: return "Box2D BodyId is invalid";

				// ==== OPENGL EXCEPTIONS ====
			case ERROR_GLEWInitFailed: return "GLEWInit failed";
			default: return "Unkown error code";
		}
	}
}