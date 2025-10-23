#pragma once
#include "ComponentManagers/GenericComponentManager.h"
#include <chrono>
#include <iostream>
#include "Smasher/Base.h"
#include "IComponentManager.h"
#include "IComponent.h"
#include "plf_colony.h"

namespace Smasher {
	template<class T>
	GenericComponentManager<T>::GenericComponentManager(Layer& state) : IComponentManager(state) {
		m_ComponentsToRemove.reserve(64); // Arbitrary can be improved upon later
	}

	template<class T>
	void GenericComponentManager<T>::PreUpdate(Millisecond delta) {
		RemoveMarkedComponents();
	}

	template<class T>
	void GenericComponentManager<T>::Update(Millisecond delta) {
		if constexpr (HasStaticUpdateComponent<T>) {
			for (T& itr : m_Components) {
				T::StaticUpdateComponent(itr, delta);
			}
		}
	}

	template<class T>
	void GenericComponentManager<T>::Render(sf::RenderWindow& rWindow) {
		if constexpr (HasStaticRenderComponent<T>) {
			for (T& itr : m_Components) {
				T::StaticRenderComponent(itr, rWindow);
			}
		}
	}

	template<class T>
	template<typename... Args>
	T& GenericComponentManager<T>::AddComponent(Entity& rEntity, Args&&... args) {
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
	void GenericComponentManager<T>::RemoveComponent(IComponent& rComponentInterface) {
		T& rComponent = static_cast<T&>(rComponentInterface);
		if (rComponent.GetStatus() != ComponentStatus::VALID) {
			return;
		}
		typename plf::colony<T>::iterator* pItr = GetComponentIterator<T>(rComponent);
		SetComponentStatus(rComponent, ComponentStatus::INVALID);
		m_ComponentsToRemove.emplace_back(pItr);
	}

	template<class T>
	void GenericComponentManager<T>::RemoveMarkedComponents() {
		for (auto& itr : m_ComponentsToRemove) {
			typename plf::colony<T>::iterator* pCompItr = itr;
			m_Components.erase(*pCompItr);
			delete pCompItr;
		}
		m_ComponentsToRemove.clear();
	}
}