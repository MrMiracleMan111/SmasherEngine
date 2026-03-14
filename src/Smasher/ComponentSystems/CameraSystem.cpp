#include "Smasher/ComponentSystems/CameraSystem.h"
#include "Smasher/ErrorCodes.h"

namespace Smasher {
	namespace CameraSystem {
		ErrorCode Initialize(entt::registry& registry) {
			return ERROR_NoError;
		}
		ErrorCode Teardown(entt::registry& registry) {
			return ERROR_NoError;
		}
	}
}