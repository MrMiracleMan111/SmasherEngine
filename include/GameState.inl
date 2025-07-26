#pragma once
#include "GameState.h"
#include "ComponentManager.h"
#include "GenericComponentManager.h"
#include "IRenderable.h"

namespace Smasher {

	template <typename T>
	concept HasStaticInstantiateManager = requires(Smasher::GameState& arg) {
		T::StaticInstantiateManager(arg);
	};

	template <typename T>
	concept HasStaticRenderComponent = requires(sf::RenderWindow& arg) {
		{ T::HasStaticRenderComponent(arg) } -> std::same_as<void>;
	};

	template <class T>
	void GameState::LoadComponentManager()
	{
		if constexpr (HasStaticInstantiateManager<T>) {
			m_ComponentManagers.emplace(std::type_index(typeid(T)), T::StaticInstantiateManager(*this));
			if constexpr (std::is_base_of_v<IRenderable, T>) {
				auto pManager = m_ComponentManagers[std::type_index(typeid(T))];
				m_ComponentRenderableManagers.push_back(pManager.get());
			}
		}
		// Default of Component doesn't specify a Manager Type
		else {
			m_ComponentManagers.emplace(std::type_index(typeid(T)), std::make_unique<GenericComponentManager<T>>(*this));
			if constexpr (HasStaticRenderComponent<T>) {
				auto pManager = m_ComponentManagers[std::type_index(typeid(T))];
				m_ComponentRenderableManagers.push_back(pManager.get());
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
		return *pEntityObserver;
	};

	template <class T>
	ComponentManager& GameState::GetComponentManager() {
		if (m_ComponentManagers.find(std::type_index(typeid(T))) == m_ComponentManagers.end()) {
			// Lazy Load the manager
			LoadComponentManager<T>();
		}
		return *m_ComponentManagers[std::type_index(typeid(T))].get();
	}
}