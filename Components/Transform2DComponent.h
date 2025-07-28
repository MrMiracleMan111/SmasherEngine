#pragma once
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include "Base.h"
#include "IComponent.h"

namespace Smasher {
	class Transform2DComponent : public IComponent, sf::Transformable {
		Transform2DComponent() = delete;

		Transform2DComponent(Smasher::Entity& entity) :
			Smasher::IComponent(entity) {
		};

		Transform2DComponent(Smasher::Entity& entity, sf::Transform transform) :
			Smasher::IComponent(entity), m_Transform(transform) {
			UpdateRotationCache(transform);
		};
		Transform2DComponent& operator=(Transform2DComponent&&) = default; // So that parent move assignment is called
	};
}