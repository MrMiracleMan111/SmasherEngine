#pragma once
namespace Smasher {
	template<class T>
	inline BaseComponentManager<T>::BaseComponentManager(GameState& state) : IComponentManager(state) {
		m_ComponentsToRemove.reserve(64); // Arbitrary can be improved upon later
	}

	template<class T>
	template<typename... Args>
	inline T& BaseComponentManager<T>::AddComponent(Entity& rEntity, Args&&... args) {
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

	template<class T>
	inline void BaseComponentManager<T>::RemoveComponent(IComponent& rComponentInterface) {
		T& rComponent = static_cast<T&>(rComponentInterface);
		if (rComponent.GetStatus() != ComponentStatus::VALID) {
			return;
		}
		typename plf::colony<T>::iterator* pItr = GetComponentIterator<T>(rComponent);
		SetComponentStatus(rComponent, ComponentStatus::INVALID);
		m_ComponentsToRemove.emplace_back(pItr);
	}

	template<class T>
	inline void BaseComponentManager<T>::RemoveMarkedComponents() {
		for (auto& itr : m_ComponentsToRemove) {
			typename plf::colony<T>::iterator* pCompItr = itr;
			m_Components.erase(*pCompItr);
			delete pCompItr;
		}
		m_ComponentsToRemove.clear();
	}

	template<class T>
	inline void BaseComponentManager<T>::PreUpdate(Millisecond delta) {
		RemoveMarkedComponents();
	}

	template<class T>
	inline void BaseComponentManager<T>::Update(Millisecond delta) {
		UpdateComponents(delta);
	}

	template<class T>
	inline void BaseComponentManager<T>::Render(sf::RenderWindow& rWindow) {
		RenderComponents(rWindow);
	}
}