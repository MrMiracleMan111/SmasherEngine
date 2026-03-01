#pragma once
#include "entt/entity/registry.hpp"
#include "Smasher/Base.h"

namespace Smasher {
	namespace CameraSystem {
		struct Context {
			std::vector<entt::entity> cameras;
		};

		struct Component {

		};

		entt::entity ConstructCamera();
		SMASHER_API const ErrorCode Initialize(entt::registry& registry);
		SMASHER_API const ErrorCode Teardown(entt::registry& registry);
	}
}