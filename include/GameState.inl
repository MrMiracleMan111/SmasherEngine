#pragma once
#include "GameState.h"
#include "IComponentManager.h"
#include "GenericComponentManager.h"
#include "Entity.h"


namespace Smasher {
	/*
		Instantiates a Component Manager to handle the provided component type. Components
		may handle manager instantion through their StaticInstatiateManager<T>() method.
	
		If Component type T doesn't specify a Component Manager type, the GenericComponentManager
	*/
	template <class T>
	void GameState::LoadComponentManager()
	{
		static_assert(IComponentType<T>, "T should be derived from IComponent");

		// Component specifies a manager to use
		if constexpr (HasStaticInstantiateManager<T>) {
			using ManagerType = std::decay_t<decltype(T::StaticInstantiateManager(*this))>::type;
			static_assert(ComponentManagerHasAddComponent<ManagerType, T>, "ComponentManager is missing AddComponent method");
			static_assert(std::derived_from<ManagerType, IComponentManager>, "StaticInstantiateManager return type must derive from IComponentManager");

			m_ComponentManagers.emplace(std::type_index(typeid(T)), T::StaticInstantiateManager(*this));
			auto& rManagerPtr = m_ComponentManagers[std::type_index(typeid(T))];

			// Was Update method overriden?
			if constexpr (!std::same_as<ManagerType::Update, decltype(&IComponentManager::Update)>) {
				m_ComponentManagersWithUpdate.push_back(rManagerPtr.get());
			}

			// Was Render method overriden?
			if constexpr (!std::same_as<ManagerType::Render, decltype(&IComponentManager::Render)>) {
				m_ComponentManagersWithRender.push_back(rManagerPtr.get());
			}
		}
		// Use a GenericComponentManager<T> if Component doesn't specify a manager to use
		else {
			// Move static_assert outside the class
			m_ComponentManagers.emplace(std::type_index(typeid(T)), std::make_unique<GenericComponentManager<T>>(*this));
			static_assert(ComponentManagerHasAddComponent<GenericComponentManager<T>, T>, "ComponentManager is missing AddComponent method");
			auto& rManagerPtr = m_ComponentManagers[std::type_index(typeid(T))];

			if constexpr (HasStaticRenderComponent<T>) {
				m_ComponentManagersWithRender.push_back(rManagerPtr.get());
			}

			if constexpr (HasStaticUpdateComponent<T>) {
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

	template <class T>
	IComponentManager& GameState::GetComponentManager() {
		if (m_ComponentManagers.find(std::type_index(typeid(T))) == m_ComponentManagers.end()) {
			// Lazy Load the manager
			LoadComponentManager<T>();
		}
		return *m_ComponentManagers[std::type_index(typeid(T))].get();
	}
}