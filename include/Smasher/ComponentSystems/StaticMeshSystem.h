#pragma once
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <entt/entity/registry.hpp>
#include "Smasher/Base.h"
#include "Smasher/Resources.h"
#include "Smasher/Exceptions.h"

namespace Smasher {
	namespace StaticMeshSystem {
		struct Context {
			SDL_GPUBuffer* buffer;
		};
		struct Component {
			std::shared_ptr<StaticMeshResource> meshResource;
		};

		SMASHER_API ErrorCode Initialize(entt::registry& registry);
		SMASHER_API ErrorCode Teardown(entt::registry& registry);
		SMASHER_API Expected<std::reference_wrapper<Component>> AddComponent(entt::registry& registry, entt::entity entity, std::shared_ptr<StaticMeshResource> meshResource);
	}
}
