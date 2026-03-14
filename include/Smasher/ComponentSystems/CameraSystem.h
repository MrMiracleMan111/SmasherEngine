#pragma once
#include "entt/entity/registry.hpp"
#include "Smasher/Base.h"

namespace Smasher {
	namespace CameraSystem {
		struct SMASHER_API Context {
			std::vector<entt::entity> cameras;
		};

		struct SMASHER_API Component {

		};

		entt::entity ConstructCamera();
		SMASHER_API const ErrorCode Initialize(entt::registry& registry);
		SMASHER_API const ErrorCode Teardown(entt::registry& registry);
	}
}