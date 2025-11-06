#pragma once

namespace Smasher {

	template<typename... Args>
	AABBPhysicsComponent& AABBPhysicsComponentManager::AddComponent(Entity& rEntity, Args&&... args) {
		return _AddComponent(rEntity);
	}
}