#pragma once
#include <SFML/Graphics.hpp>
#include "Base.h"

namespace Smasher {
	template<class T>
	class GenericRenderableComponentManager : public GenericRenderableComponentManager<T> {
		void Render(sf::RenderWindow& window) {};
	};
}