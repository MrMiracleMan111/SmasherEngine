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
	static_assert(std::is_base_of_v<IComponent, T>, "T should be derived from IComponent");

		friend class Entity;
	public:
		BaseComponentManager(GameState& state) : IComponentManager(state) {
			m_ComponentsToRemove.reserve(64); // Arbitrary can be improved upon later
		};
		BaseComponentManager() = delete;
		BaseComponentManager(const BaseComponentManager&) = delete;
		BaseComponentManager(BaseComponentManager&&) = delete;
		BaseComponentManager& operator=(const BaseComponentManager&) = delete;
		BaseComponentManager& operator=(BaseComponentManager&&) = delete;
		virtual ~BaseComponentManager() = default;

		virtual void PreUpdateComponents() {};
		virtual void UpdateComponents(Millisecond delta) {};
		virtual void RenderComponents(sf::RenderWindow& rWindow) {};

		template<typename... Args>
		T& AddComponent(Entity& rEntity, Args&&... args) {
			std::size_t index = m_Components.size();
			auto itr = m_Components.emplace(std::forward<Args>(args)...);
			T& rComponent = *itr;
			SetComponentStatus(rComponent, ComponentStatus::VALID);
			SetComponentEntity(rComponent, rEntity);
			SetComponentManager(rComponent, *this);
			SetComponentIterator<T>(rComponent, new typename plf::colony<T>::iterator(itr));
			CallOnAddComponent(rComponent); // called when component has been initialiazed and added

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

	protected:
		plf::colony<T> m_Components;
	private:
		void RemoveMarkedComponents() {
			for (auto& itr : m_ComponentsToRemove) {
				typename plf::colony<T>::iterator* pCompItr = itr;
				m_Components.erase(*pCompItr);
				delete pCompItr;
			}
			m_ComponentsToRemove.clear();
		}

		void PreUpdate(Millisecond delta) override {
			RemoveMarkedComponents();
		}

		void Update(Millisecond delta) override {
			UpdateComponents(delta);
		}

		void Render(sf::RenderWindow& rWindow) override {
			RenderComponents(rWindow);
		}

		std::vector<typename plf::colony<T>::iterator*> m_ComponentsToRemove; // Components to be removed
	};
}