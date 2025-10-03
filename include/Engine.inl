#include <memory>
#include "Engine.h"
#include "Layer.h"

namespace Smasher {
	template<class T, typename... Args>
	T& Engine::PushLayer(Args&&... componentArgs) {
		if (HasLayer<T>()) {
			std::string exceptionMessage = std::format("Engine already has State of type {}", typeid(T).name());
			throw Exceptions::LayerDuplicate(exceptionMessage);
		}
		std::type_index index = std::type_index(typeid(T));
		auto pLayer = std::make_unique<T>(*this, std::forward<Args>(componentArgs)...);
		T& rLayer = *pLayer;
		m_LayerStack.emplace(m_LayerStack.begin(), index, std::move(pLayer));
		rLayer.Init();
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
		return std::find_if(m_LayerStack.begin(), m_LayerStack.end(),
			[](auto& itr) {
				return std::type_index(typeid(T)) == itr.first;
			}) != m_LayerStack.end();
	}
}