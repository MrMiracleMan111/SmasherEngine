#pragma once
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include "Base.h"
#include "IComponent.h"

namespace Smasher {
	class Transform2DComponent : public IComponent, public sf::Transformable {
	public:
		Transform2DComponent() : IComponent(), sf::Transformable() {}
	};
}