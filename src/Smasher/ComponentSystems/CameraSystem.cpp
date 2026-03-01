#include "Smasher/ComponentSystems/CameraSystem.h"
#include "Smasher/ErrorCodes.h"

namespace Smasher {
	namespace CameraSystem {
		const ErrorCode Initialize(entt::registry& registry) {
			return ERROR_NoError;
		}
		const ErrorCode Teardown(entt::registry& registry) {
			return ERROR_NoError;
		}
	}
}