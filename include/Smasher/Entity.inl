#pragma once
#include "Smasher/Entity.h"
#include "Smasher/Layer.h"
namespace Smasher {
	template<IComponentType T, typename... Args>
	T& Entity::AddComponent(Args&&... componentArgs) {

		if (HasComponent<T>()) {
			std::string exceptionMessage = std::format("Entity already has component of type {}", typeid(T).name());
			throw Exceptions::EntityDuplicateComponent(exceptionMessage);
		}

		if constexpr (HasStaticInstantiateManager<T>) {
			// Use Custom Manager
			using ManagerType = typename decltype(T::StaticInstantiateManager(m_LayerRef.get()))::element_type;

			
			ManagerType& rManager = static_cast<ManagerType&>(m_LayerRef.get().GetComponentManager<T>());
			T& rComponent = rManager.AddComponent(*this, std::forward<Args>(componentArgs)...);
			IComponent& rComponentInterface = static_cast<IComponent&>(rComponent);
			m_ComponentsByType.emplace(std::type_index(typeid(T)), rComponentInterface);
			return rComponent;
		}
		else {
			// Use Generic
    		using ManagerType = GenericComponentManager<T>;
            ManagerType& rManager = static_cast<ManagerType&>(m_LayerRef.get().GetComponentManager<T>());
			T& rComponent = rManager.AddComponent(*this, std::forward<Args>(componentArgs)...);
			IComponent& rComponentInterface = static_cast<IComponent&>(rComponent);
			m_ComponentsByType.emplace(std::type_index(typeid(T)), rComponentInterface);
			return rComponent;
		}
	}

	template<class T>
	void Entity::RemoveComponent() {
		if (!HasComponent<T>()) {
			throw Exceptions::EntityComponentNotFound(std::format("Could not find component for entity {}", (uint64_t)(m_UUID)));
		}
		const std::type_index index = std::type_index(typeid(T));
		std::reference_wrapper<IComponent>& rComponentPtr = m_ComponentsByType.at(index);

		IComponentManager& manager = m_LayerRef.get().template GetComponentManager<T>();
		manager.RemoveComponent(rComponentPtr.get());
		m_ComponentsByType.erase(index);
	}


	template<class T>
	T& Entity::GetComponent() const {
		const std::type_index index = std::type_index(typeid(T));
		auto itr = m_ComponentsByType.find(index);
		if (itr == m_ComponentsByType.end()) {
			std::string exceptionMessage = std::format("Entity has no component of type {}", typeid(T).name());
			throw Exceptions::EntityComponentNotFound(exceptionMessage);
		}
		return static_cast<T&>(itr->second.get());
	}
}