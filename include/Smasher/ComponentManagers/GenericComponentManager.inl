#pragma once
#include "Smasher/ComponentManagers/GenericComponentManager.h"
#include <chrono>
#include <iostream>
#include "Smasher/Base.h"
#include "Smasher/IComponentManager.h"
#include "Smasher/IComponent.h"
#include "Smasher/plf_colony.h"

namespace Smasher {
	template<class T>
	GenericComponentManager<T>::GenericComponentManager(Layer &state) : IComponentManager(state) {
		m_ComponentsToRemove.reserve(64); // Arbitrary can be improved upon later
	}

	template<typename T>
	GenericComponentManager<T>::~GenericComponentManager() {
		m_Components.clear();
	}

	template<class T>
	void GenericComponentManager<T>::PreUpdate(Millisecond delta) {
		RemoveMarkedComponents();
	}

	template<class T>
	void GenericComponentManager<T>::Update(Millisecond delta) {
		if constexpr (HasStaticUpdateComponent<T>) {
			for (T &itr : m_Components) {
				T::StaticUpdateComponent(itr, delta);
			}
		}
	}

	template<class T>
	void GenericComponentManager<T>::Render(sf::RenderWindow &window) {
		if constexpr (HasStaticRenderComponent<T>) {
			for (T &itr : m_Components) {
				T::StaticRenderComponent(itr, window);
			}
		}
	}

	template<class T>
	template<typename... Args>
	T& GenericComponentManager<T>::AddComponent(Entity &entity, Args&&... args) {
		std::size_t index = m_Components.size();
		auto itr = m_Components.emplace(std::forward<Args>(args)...);
		T &rComponent = *itr;
		SetComponentStatus(rComponent, ComponentStatus::VALID);
		SetComponentEntity(rComponent, entity);
		SetComponentManager(rComponent, *this);
		SetComponentIterator<T>(rComponent, new typename plf::colony<T>::iterator(itr));
		CallOnAddComponent(rComponent); // called when component has been initialiazed and added

		return rComponent;
	}

	template<class T>
	void GenericComponentManager<T>::RemoveComponent(IComponent &componentInterface) {
		T &component = static_cast<T&>(componentInterface);
		if (component.GetStatus() != ComponentStatus::VALID) {
			return;
		}
		typename plf::colony<T>::iterator *pItr = GetComponentIterator<T>(component);
		SetComponentStatus(component, ComponentStatus::INVALID);
		m_ComponentsToRemove.emplace_back(pItr);
	}

	template<class T>
	void GenericComponentManager<T>::RemoveMarkedComponents() {
		for (auto &itr : m_ComponentsToRemove) {
			typename plf::colony<T>::iterator *pCompItr = itr;
			m_Components.erase(*pCompItr);
			delete pCompItr;
		}
		m_ComponentsToRemove.clear();
	}
}