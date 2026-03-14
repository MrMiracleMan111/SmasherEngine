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
		SMASHER_API ErrorCode Initialize(entt::registry& registry);
		SMASHER_API ErrorCode Teardown(entt::registry& registry);
	}
}