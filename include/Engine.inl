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
		auto pState = std::make_unique<T>(*this, std::forward<Args>(componentArgs)...);
		T& rState = *pState;
		m_LayerByType.insert({ index, std::move(pState) });
		rState.Init();
		return rState;
	}

	template<class T>
	T& Engine::GetLayer() const {
		auto itr = m_LayerByType.find(std::type_index(typeid(T)));
		if (itr == m_LayerByType.end()) {
			std::string exceptionMessage = std::format("Engine has no Layer of type {}", typeid(T).name());
			throw Exceptions::LayerNotFound(exceptionMessage);
		}
		return static_cast<T&>(*itr->second.get());
	}

	template<class T>
	bool Engine::HasLayer() const {
		return m_LayerByType.find(std::type_index(typeid(T))) != m_LayerByType.end();
	}
}