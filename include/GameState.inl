#pragma once
#include "GameState.h"
#include "IComponentManager.h"
#include "GenericComponentManager.h"
#include "Entity.h"


namespace Smasher {
	/*
		Instantiates a Component Manager to handle the provided component type. Components
		may handle manager instantion through their StaticInstatiateManager<T>() method.
	
		If Component type T doesn't specify a Component type with a StaticInstantiateManager method, the GenericComponentManager
	*/
	template <class ComponentType>
	void GameState::LoadComponentManager()
	{
		static_assert(IComponentType<ComponentType>, "T should be derived from IComponent");

		// Component specifies a manager to use
		if constexpr (HasStaticInstantiateManager<ComponentType>) {
			using ManagerType = typename decltype(ComponentType::StaticInstantiateManager(*this))::element_type;
			static_assert(ComponentManagerHasAddComponent<ManagerType, ComponentType>, "ComponentManager is missing AddComponent method");
			static_assert(std::derived_from<ManagerType, IComponentManager>, "StaticInstantiateManager return type must derive from IComponentManager");

			m_ComponentManagers.emplace(std::type_index(typeid(ComponentType)), ComponentType::template StaticInstantiateManager(*this));
			auto& rManagerPtr = m_ComponentManagers[std::type_index(typeid(ComponentType))];

			// Was Update method overriden?
			if constexpr (HasUpdateCapability<ManagerType>) {
				if constexpr (!std::is_same<decltype(&ManagerType::Update), decltype(&IComponentManager::Update)>::value) {
					m_ComponentManagersWithUpdate.push_back(rManagerPtr.get());
				}
			}

			// Was Render method overriden?
			if constexpr (HasRenderCapability<ManagerType>) {
				if constexpr (!std::is_same<decltype(&ManagerType::Render), decltype(&IComponentManager::Render)>::value) {
					m_ComponentManagersWithRender.push_back(rManagerPtr.get());
				}
			}
		}
		// Use a GenericComponentManager<T> if Component doesn't specify a manager to use
		else {
			// Move static_assert outside the class
			m_ComponentManagers.emplace(std::type_index(typeid(ComponentType)), std::make_unique<GenericComponentManager<ComponentType>>(*this));
			static_assert(ComponentManagerHasAddComponent<GenericComponentManager<ComponentType>, ComponentType>, "ComponentManager is missing AddComponent method");
			auto& rManagerPtr = m_ComponentManagers[std::type_index(typeid(ComponentType))];

			if constexpr (HasStaticRenderComponent<ComponentType>) {
				m_ComponentManagersWithRender.push_back(rManagerPtr.get());
			}

			if constexpr (HasStaticUpdateComponent<ComponentType>) {
				 m_ComponentManagersWithUpdate.push_back(rManagerPtr.get());
			}
		}
	}

	template <class T, typename... Args>
	T& GameState::AddEntity(Args&&... componentArgs) {
		static_assert(std::is_base_of<Entity, T>::value, "T must inherit from Entity");
		auto pEntity = std::make_unique<T>(*this, UUID::GetUUID(), std::forward<Args>(componentArgs)...);
		T* pEntityObserver = pEntity.get();
		UUID uuid = pEntity->GetUUID(); // To avoid possible invalidation during std::move(pEntity)
		m_EntityMap.insert({ uuid, std::move(pEntity) });
		pEntityObserver->Init();
		return *pEntityObserver;
	};

	template <class ComponentType>
	IComponentManager& GameState::GetComponentManager() {
		if (m_ComponentManagers.find(std::type_index(typeid(ComponentType))) == m_ComponentManagers.end()) {
			// Lazy Load the manager
			LoadComponentManager<ComponentType>();
		}
		return *m_ComponentManagers[std::type_index(typeid(ComponentType))].get();
	}
}