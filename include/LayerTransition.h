#include <typeindex>
#include "Base.h"

namespace Smasher {
	class Layer;
	enum LayerTransitionType {
		ADD,
		REMOVE,
		// MOVE (shifting layer up/down)
	};

	// Contains information about a layer transition
	struct LayerTransition {
		LayerTransition() = delete;
		LayerTransition(LayerTransitionType type, std::unique_ptr<Layer> pLayer) :
			type(type),
			pLayer(std::move(pLayer)) {
		}
		~LayerTransition() = default;
		LayerTransition(LayerTransition&&) = default;
		LayerTransition(const LayerTransition&) = delete;
		LayerTransition& operator =(LayerTransition&&) = default;
		LayerTransition& operator =(const LayerTransition&) = delete;

		const LayerTransitionType type;
		std::unique_ptr<Layer> pLayer;

		struct LayerTransitionAdd {
			std::type_index index;
		};

		struct LayerTransitionRemove {
			std::type_index index;
		};

		union {
			LayerTransitionAdd addTransition;
			LayerTransitionRemove removeTransition;
		};
	};
}
