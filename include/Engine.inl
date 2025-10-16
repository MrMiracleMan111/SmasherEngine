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

	template<class T>
	T& Engine::GetLayer() const {
		auto itr = std::find_if(m_LayerStack.begin(), m_LayerStack.end(),
			[](auto& itr) {
				return std::type_index(typeid(T)) == itr.first;
			});

		if (itr == m_LayerStack.end()) {
			std::string exceptionMessage = std::format("Engine has no Layer of type {}", typeid(T).name());
			throw Exceptions::LayerNotFound(exceptionMessage);
		}

		return static_cast<T&>(*(itr->second.get()));
	}

	template<class T>
	bool Engine::HasLayer() const {
		return !m_LayerStack.empty() && std::find_if(m_LayerStack.begin(), m_LayerStack.end(),
			[](auto& itr) {
				return std::type_index(typeid(T)) == itr.first;
			}) != m_LayerStack.end();
	}
}