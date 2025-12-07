#pragma once

namespace Smasher {
	template<typename... Args>
	UIPanelComponent& UIPanelComponentManager::AddComponent(Entity &entity, Args&&... args) {
		UIPanelComponent &component = BaseComponentManager<UIPanelComponent>::AddComponent(entity, std::forward<Args>(args)...);
		component.InitGLObjects();
		
		return component;
	}
}