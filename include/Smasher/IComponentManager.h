#pragma once
#include <memory>
#include "Smasher/plf_colony.h"
#include <SFML/Window.hpp>
#include "Smasher/IComponent.h"
#include "Smasher/Base.h"

namespace Smasher {
	class Layer;

	//////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief Interface used by all component managers.
	/// 
	/// All Component Managers must inherit this interface
	//////////////////////////////////////////////////////////////////////////////////////////////////
	class IComponentManager {
	public:
		IComponentManager() = delete;
		virtual ~IComponentManager() {};
		IComponentManager(Layer &layer) : m_Layer(layer) {}
		virtual void PreUpdate(Millisecond delta) {};
		virtual void Update(Millisecond delta) {}; // Override this to enable component updating
		virtual void PostUpdate(Millisecond delta) {}; // Override this to enable component updating
		virtual void Render(sf::RenderWindow &window) {}; // Override this to enable component rendering
		virtual void RemoveComponent(IComponent &component) = 0;

		Layer& GetLayer() { return m_Layer; }

	protected:
		void SetComponentStatus(IComponent &component, ComponentStatus status) { component.SetStatus(status); }
		void SetComponentEntity(IComponent &component, Entity &entity) { component.SetEntity(entity); }
		void SetComponentManager(IComponent &component, IComponentManager &manager) { component.SetManager(manager); }
		void CallOnAddComponent(IComponent &component) { component.OnAddComponent(); } // called when component has been initialiazed and added

		template<class T>
		typename plf::colony<T>::iterator* GetComponentIterator(T& component) {
			return component.template GetIterator<T>();
		}

		template<class T>
		void SetComponentIterator(T& component, typename plf::colony<T>::iterator *itrPtr) {
			component.template SetIterator<T>(itrPtr);
		}

	private:
		Layer& m_Layer;
	};
}