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
		IComponentManager(Layer& layer) : m_Layer(layer) {}
		virtual void PreUpdate(Millisecond delta) {};
		virtual void Update(Millisecond delta) {}; // Override this to enable component updating
		virtual void Render(sf::RenderWindow& rWindow) {}; // Override this to enable component rendering
		virtual void RemoveComponent(IComponent& component) = 0;

		Layer& GetLayer() { return m_Layer; }

		Layer& m_Layer;
	protected:
		void SetComponentStatus(IComponent& rComponent, ComponentStatus status) { rComponent.SetStatus(status); }
		void SetComponentEntity(IComponent& rComponent, Entity& rEntity) { rComponent.SetEntity(rEntity); }
		void SetComponentManager(IComponent& rComponent, IComponentManager& rManager) { rComponent.SetManager(rManager); }
		void CallOnAddComponent(IComponent& rComponent) { rComponent.OnAddComponent(); } // called when component has been initialiazed and added

		template<class T>
		typename plf::colony<T>::iterator* GetComponentIterator(T& rComponent) {
			return rComponent.template GetIterator<T>();
		}

		template<class T>
		void SetComponentIterator(T& rComponent, typename plf::colony<T>::iterator* itrPtr) {
			rComponent.template SetIterator<T>(itrPtr);
		}
	};
}