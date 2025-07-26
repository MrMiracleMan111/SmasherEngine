#pragma once
#include "Entity.h"
#include "GameState.h"
namespace Smasher {
	template<class T, typename... Args>
	T& Entity::AddComponent(Args&&... componentArgs) {
		static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

		if (HasComponent<T>()) {
			std::string exceptionMessage = std::format("Entity already has component of type {}", typeid(T).name());
			throw Exceptions::EntityDuplicateComponent(exceptionMessage);
		}

		auto& manager = m_GameState.template GetComponentManager<T>();
		auto pComponent = std::make_unique<T>(*this, std::forward<Args>(componentArgs)...);
		std::reference_wrapper<std::unique_ptr<Component>> refComponentPtr(manager.AddComponent(std::move(pComponent)));
		m_ComponentsByType.insert({ std::type_index(typeid(T)), refComponentPtr });
		return static_cast<T&>(*(refComponentPtr.get().get()));
	}

	template<class T>
	void Entity::RemoveComponent() {
		if (!HasComponent<T>()) {
			throw Exceptions::EntityComponentNotFound(std::format("Could not find component for entity {}", (uint64_t)(m_UUID)));
		}
		std::type_index index = std::type_index(typeid(T));
		std::reference_wrapper<std::unique_ptr<Component>>& refComponentPtr = m_ComponentsByType.at(index);

		auto& manager = m_GameState.template GetComponentManager<T>();
		manager.RemoveComponent(refComponentPtr.get());
		m_ComponentsByType.erase(index);
	}


	template<class T>
	T& Entity::GetComponent() {
		auto itr = m_ComponentsByType.find(std::type_index(typeid(T)));
		if (itr == m_ComponentsByType.end()) {
			std::string exceptionMessage = std::format("Entity has no component of type {}", typeid(T).name());
			throw Exceptions::EntityComponentNotFound(exceptionMessage);
		}
		return static_cast<T&>(*itr->second.get());
	}
}