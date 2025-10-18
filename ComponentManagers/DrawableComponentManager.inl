#pragma once

namespace Smasher {
	template<typename... Args>
	DrawableComponent& DrawableComponentManager::AddComponent(Entity& entity, Args&&... args) {
		DrawableComponent& component = BaseComponentManager<DrawableComponent>::AddComponent(entity, std::forward<Args>(args)...);

		component.SetShader(GetDefaultShader());

		return component;
	}
}