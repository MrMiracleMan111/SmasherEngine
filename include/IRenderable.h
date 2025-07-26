#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "Base.h"

namespace Smasher {
	class IRenderable {
	public:
		virtual void Render(sf::RenderWindow& window) = 0;
	};
}