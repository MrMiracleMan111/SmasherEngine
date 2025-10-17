#pragma once
#include <memory>
//#include "Engine.h"
//#include "Layer.h"

namespace Smasher {
	class Layer;
	class BaseLayer;

	template<class T, typename... Args>
	T& Engine::PushLayer(Args&&... componentArgs) {
		if (HasLayer<T>()) {
			std::string exceptionMessage = std::format("Engine already has State of type {}", typeid(T).name());
			throw Exceptions::LayerDuplicate(exceptionMessage);
		}
		auto pLayer = std::make_unique<T>(*this, std::forward<Args>(componentArgs)...);
		T& rLayer = *pLayer;

		LayerTransition& transition = m_LayerTransitions.emplace_back(LayerTransitionType::ADD, std::move(pLayer));
		transition.addTransition.index = std::type_index(typeid(T));

		// Immediately add layer if engine is not running
		if (!m_RunningAtomic) {
			HandleLayerTransitions();
		}

		return rLayer;
	}

	// Creates layer and adds it to transition list
	template<class T>
	void Engine::PopLayer() {
		if (std::type_index(typeid(T)) == std::type_index(typeid(BaseLayer))) {
			throw Exceptions::CannotRemoveBaseLayer("Removing BaseLayer is not permitted.");
		}

		if (!HasLayer<T>()) {
			throw Exceptions::LayerNotFound("This layer is not on the stack");
		}

		// If LayerTransition takes ownership of the std::unique_ptr<Layer>,
		// all future calls to GetLayer<T> for that layer will be messed up
		LayerTransition& transition = m_LayerTransitions.emplace_back(LayerTransitionType::REMOVE, nullptr);
		transition.removeTransition.index = std::type_index(typeid(T));

		// Immediately add layer if engine is not running
		if (!m_RunningAtomic) {
			HandleLayerTransitions();
		}
	}

	template<class T>
	T& Engine::GetLayer() const {
		Engine::LayerStackConstItr itr = GetLayerItr(std::type_index(typeid(T)));

		if (itr == m_LayerStack.end()) {
			std::string exceptionMessage = std::format("Engine has no Layer of type {}", typeid(T).name());
			throw Exceptions::LayerNotFound(exceptionMessage);
		}

		return static_cast<T&>(*(itr->second.get()));
	}

	template<class T>
	bool Engine::HasLayer() const {
		return GetLayerItr(std::type_index(typeid(T))) != m_LayerStack.end();
	}
}