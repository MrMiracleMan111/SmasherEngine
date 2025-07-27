#pragma once
#include "Base.h"
#include "IComponentManager.h"

namespace Smasher {
	class Entity;
	class GameState;

	/*
	* 
	*  This class is meant to be inherited
	* 
	* 
	*/
	template <class T>
	class SMASHER_API BaseComponentManager : public IComponentManager {
		friend class Entity;
	public:
		BaseComponentManager(GameState& state) : IComponentManager(state) {};
		BaseComponentManager() = delete;
		BaseComponentManager(const BaseComponentManager&) = delete;
		BaseComponentManager(BaseComponentManager&&) = delete;
		BaseComponentManager& operator=(const BaseComponentManager&) = delete;
		BaseComponentManager& operator=(BaseComponentManager&&) = delete;
		virtual ~BaseComponentManager() = default;

		virtual void PreUpdateComponents() {};
		virtual void UpdateComponents(Millisecond delta) {};
		virtual void RenderComponents(sf::RenderWindow& rWindow) {};

	protected:
		IComponent& AddComponent(IComponent&& xComponent) override {
			T* pComponent = dynamic_cast<T*>(&xComponent);
			if (pComponent == nullptr) {
				throw Exceptions::ComponentDowncastFailed(std::format("Component not of type {}", typeid(T).name()));
			}
			SetComponentStatus(*pComponent, ComponentStatus::VALID);
			m_Components.emplace_back(std::move(*pComponent));
			return m_Components.back();
		}

		void RemoveComponent(IComponent& rComponent) override {
			if (rComponent.GetStatus() != ComponentStatus::VALID) {
				return;
			}

			SetComponentStatus(rComponent, ComponentStatus::INVALID);
			m_ComponentsToRemove.emplace_back(rComponent);
		}

	private:
		void PreUpdate(Millisecond delta) override {
			RemoveMarkedComponents();
		}

		void Update(Millisecond delta) override {
			UpdateComponents(delta);
		}

		void Render(sf::RenderWindow& rWindow) override {
			RenderComponents(rWindow);
		}

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