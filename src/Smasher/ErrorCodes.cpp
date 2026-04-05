#include "Smasher/ErrorCodes.h"

namespace Smasher {
	const char* GetErrorMessage(ErrorCode code) {
		switch (code) {
			// ==== NO ERROR ====
		case ERROR_NoError: return "Success, no errors were thrown";

		case ERROR_ExpectedHasError: return "Expected<T> Has Error";
		case ERROR_EngineNotInitialized: return "Engine Not Initialized";
		case ERROR_EngineIsInHeadlessMode: return "Engine Is In Headless Mode";
		case ERROR_EngineFailedtoInitializeGPU: return "Engien could not initialize the SDL GPU Device";

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

			// ==== JOBS EXCEPTIONS ====
		case ERROR_NotEnoughThreads: return "Could not create enough threads, check MIN_THREAD_COUNT in EngineConfig.h";
		case ERROR_NoJobsAvailable: return "No jobs were available";
		case ERROR_JobHasNoParents: return "Job has no parents";
		case ERROR_SyncJobCannotDependOnSyncJob: return "Synchronous jobs cannot depend on other synchronous jobs";
		
		default: return "Unkown error code";
		}
	}
}