#include "Smasher/ComponentSystems/StaticMeshSystem.h"
#include "Smasher/ComponentSystems/TransformSystem.h"
#include "Smasher/ErrorCodes.h"

namespace Smasher {
	namespace StaticMeshSystem {
		ErrorCode Initialize(entt::registry& registry) {
			if (registry.ctx().contains<Context>()) {
				return ERROR_SystemAlreadyInitialized;
			}

			registry.ctx().emplace<Context>();

			return ERROR_NoError;
		}
		ErrorCode Teardown(entt::registry& registry) {
			return ERROR_NoError;
		}

		Expected<std::reference_wrapper<Component>> AddComponent(entt::registry& registry, entt::entity entity, std::shared_ptr<StaticMeshResource> meshResource) {
			assert(registry.all_of<TransformSystem::Component>(entity) && "StaticMeshSystem::Component requires TransformSystem::Component");

			if (!registry.ctx().contains<Context>()) {
				return Expected<std::reference_wrapper<Component>>::Error(ERROR_SystemNotInitialized);
			}

			Context& ctx = registry.ctx().get<Context>();
			Component& component = registry.emplace<Component>(entity);
			component.meshResource = meshResource;
			return std::ref(component);
		}
	}
}
