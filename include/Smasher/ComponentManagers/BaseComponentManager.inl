#pragma once
namespace Smasher {
	template<class T>
	BaseComponentManager<T>::BaseComponentManager(Layer &state) : IComponentManager(state) {
		m_ComponentsToRemove.reserve(64); // Arbitrary can be improved upon later
	}

	template<typename T>
	BaseComponentManager<T>::~BaseComponentManager() {
		m_Components.clear();
	}

	template<class T>
	template<typename... Args>
	T& BaseComponentManager<T>::AddComponent(Entity &entity, Args&&... args) {
		std::size_t index = m_Components.size();
		auto itr = m_Components.emplace(std::forward<Args>(args)...);
		T &component = *itr;
		SetComponentStatus(component, ComponentStatus::VALID);
		SetComponentEntity(component, entity);
		SetComponentManager(component, *this);
		SetComponentIterator<T>(component, new typename plf::colony<T>::iterator(itr));
		CallOnAddComponent(component); // called when component has been initialiazed and added

		return component;
	}

	template<class T>
	void BaseComponentManager<T>::RemoveComponent(IComponent &componentInterface) {
		T &component = static_cast<T&>(componentInterface);
		if (component.GetStatus() != ComponentStatus::VALID) {
			return;
		}
		typename plf::colony<T>::iterator *pItr = GetComponentIterator<T>(component);
		SetComponentStatus(component, ComponentStatus::INVALID);
		m_ComponentsToRemove.emplace_back(pItr);
	}

	template<class T>
	void BaseComponentManager<T>::RemoveMarkedComponents() {
		for (auto &itr : m_ComponentsToRemove) {
			typename plf::colony<T>::iterator *pCompItr = itr;
			m_Components.erase(*pCompItr);
			delete pCompItr;
		}
		m_ComponentsToRemove.clear();
	}

	template<class T>
	void BaseComponentManager<T>::PreUpdate(Millisecond delta) {
		RemoveMarkedComponents();
	}

	template<class T>
	void BaseComponentManager<T>::Update(Millisecond delta) {
		UpdateComponents(delta);
	}

	template<class T>
	void BaseComponentManager<T>::Render(sf::RenderWindow& rWindow) {
		RenderComponents(rWindow);
	}
}