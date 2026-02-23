#include "entt/entity/registry.hpp"
#include "Smasher/ErrorCodes.h"
#include "Smasher/ComponentSystems/DrawableSystem.h"

namespace Smasher {
	namespace DrawableSystem {
		const ErrorCode Initialize(entt::registry& registry) {
			if (registry.ctx().contains<Context>()) {
				return ERROR_SystemAlreadyInitialized;
			}

			registry.ctx().emplace<Context>();
			return ERROR_NoError;
		}

		const ErrorCode Render(entt::registry& registry) {
			if (!registry.ctx().contains<Context>()) {
				return ERROR_SystemNotInitialized;
			}

			return ERROR_NoError;
		}

		const Expected<std::reference_wrapper<Component>> AddComponent(entt::registry& registry, entt::entity entity) {
			return std::ref(registry.emplace<Component>(entity));
		}
	}
}