#pragma once
#include "Base.h"
#include "IComponentManager.h"
#include "IComponent.h"


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
		friend class Entity;
	public:
		GenericComponentManager(GameState& state) : IComponentManager(state) {};
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
			for (T& itr : m_Components) {
				T::StaticUpdateComponent(itr, delta);
			}
		}

		void Render(sf::RenderWindow rWindow) {
			for (T& itr : m_Components) {
				T::StaticRenderComponent(itr, rWindow);
			}
		}

	protected:
		void RemoveComponent(IComponent& rComponent) override {
			if (rComponent.GetStatus() != ComponentStatus::VALID) {
				return;
			}

			SetComponentStatus(rComponent, ComponentStatus::INVALID);
			m_ComponentsToRemove.emplace_back(rComponent);
		}

		IComponent& AddComponent(IComponent&& xComponent) override {
			T* pComponent = dynamic_cast<T*>(&xComponent);
			if (pComponent == nullptr) {
				throw Exceptions::ComponentDowncastFailed(std::format("Component not of type {}", typeid(T).name()));
			}
			SetComponentStatus(*pComponent, ComponentStatus::VALID);
			m_Components.emplace_back(std::move(*pComponent));
			return m_Components.back();
		}

	private:
		void RemoveMarkedComponents() {
			for (IComponent& rComponent : m_ComponentsToRemove) {
				if (rComponent.GetStatus() == ComponentStatus::REMOVED) {
					throw Exceptions::ComponentInvalid("Component was already destroyed");
				}
				else if (rComponent.GetStatus() != ComponentStatus::INVALID) {
					throw std::logic_error("Component Status is wrong, it should be marked INVALID");
				}

				// Need to do the empty() check to avoid size_t overflow from 
				// the expression m_Components.size() - 1
				if (!m_Components.empty() && (rComponent.GetIndex() < (m_Components.size() - 1))) {
					// Move last element into this index and pop off last element
					size_t index = rComponent.GetIndex();
					rComponent = std::move(m_Components.back());
					//std::swap(rComponent, m_Components.back());
					SetComponentIndex(rComponent, index);
				}

				m_Components.pop_back();
			}
			m_ComponentsToRemove.clear();
		}

		std::vector<T> m_Components;
		std::vector<std::reference_wrapper<IComponent>> m_ComponentsToRemove; // Components to be removed
	};
}