#pragma once
#include <iostream>
#include "Base.h"
#include "IComponentManager.h"
#include "IComponent.h"
#include "plf_colony.h"

/**
	Instead of component logic being handled in the manager, this manager expects
	components to have a static method "StaticUpdateComponent" to update component.

	The effect is offloading the update code to the component to remove the need for
	new Component Managers for every component (especially useful for simple components with little logic)


	This Component Manager will also check if the template T has a "StaticUpdateComponent" before trying
	to run it. This removes need for "StaticUpdateComponent" implementation on purely
	data components (Position, Rotation, etc.) where it wouldn't do anything.
*/

namespace Smasher {
	template <class T>
	class GenericComponentManager final : public IComponentManager {
		static_assert(std::is_base_of_v<IComponent, T>, "T should be derived from IComponent");

		friend class Entity;
	public:
		GenericComponentManager(GameState& state) : IComponentManager(state) {
			m_ComponentsToRemove.reserve(64); // Arbitrary can be improved upon later
		};
		GenericComponentManager() = delete;
		GenericComponentManager(const GenericComponentManager&) = delete;
		GenericComponentManager(GenericComponentManager&&) = delete;
		GenericComponentManager& operator=(const GenericComponentManager&) = delete;
		GenericComponentManager& operator= (GenericComponentManager&&) = default;
		virtual ~GenericComponentManager() = default;

		void PreUpdate(Millisecond delta) override {
			RemoveMarkedComponents();
		}

		void Update(Millisecond delta) override {
			if constexpr (HasStaticUpdateComponent<T>) {
				for (T& itr : m_Components) {
					T::StaticUpdateComponent(itr, delta);
				}
			}
		}

		void Render(sf::RenderWindow& rWindow) override {
			if constexpr (HasStaticRenderComponent<T>) {
				for (T& itr : m_Components) {
					T::StaticRenderComponent(itr, rWindow);
				}
			}
		}

		template<typename... Args>
		T& AddComponent(Entity& rEntity, Args&&... args) {
			std::size_t index = m_Components.size();
			auto itr = m_Components.emplace(std::forward<Args>(args)...);
			T& rComponent = *itr;
			SetComponentStatus(rComponent, ComponentStatus::VALID);
			SetComponentEntity(rComponent, rEntity);
			SetComponentManager(rComponent, *this);
			SetComponentIterator<T>(rComponent, new typename plf::colony<T>::iterator(itr));

			return rComponent;
		}

		void RemoveComponent(IComponent& rComponentInterface) {
			T& rComponent = static_cast<T&>(rComponentInterface);
			if (rComponent.GetStatus() != ComponentStatus::VALID) {
				return;
			}
			typename plf::colony<T>::iterator* pItr = GetComponentIterator<T>(rComponent);
			SetComponentStatus(rComponent, ComponentStatus::INVALID);
			m_ComponentsToRemove.emplace_back(pItr);
		}

	private:
		void RemoveMarkedComponents() {
			for (auto& itr : m_ComponentsToRemove) {  
				typename plf::colony<T>::iterator* pCompItr = itr;
				m_Components.erase(*pCompItr);
				delete pCompItr;
			}
			m_ComponentsToRemove.clear();
		}

		plf::colony<T> m_Components;
		std::vector<typename plf::colony<T>::iterator*> m_ComponentsToRemove; // Components to be removed
	};
}