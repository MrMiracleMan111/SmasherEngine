#pragma once
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include "Base.h"
#include "IComponent.h"

namespace Smasher {
	class SMASHER_API Transform2DComponent : public IComponent, public sf::Transformable {

	};
}