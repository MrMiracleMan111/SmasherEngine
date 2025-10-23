#pragma once
#include <SFML/Graphics/Transformable.hpp>
#include "Smasher/Base.h"

namespace Smasher {
	template <class T>
	class Transform2DWrapper {
	public:
		Transform2DWrapper(T& caller, sf::Transformable& rTransformable) :
			m_Transformable(rTransformable), m_Caller(caller) {}

		T& SetPosition(sf::Vector2f position);

		T& SetPosition(float x, float y);

		T& SetRotation(Degrees angle);

		T& SetScale(sf::Vector2f factors);

		T& SetScale(float x, float y);

		T& SetOrigin(sf::Vector2f origin);

		T& SetOrigin(float x, float y);

		sf::Vector2f GetPosition() const { return m_Transformable.getPosition(); }

		Degrees GetRotation() const { return (Degrees)(m_Transformable.getRotation()); }

		sf::Vector2f GetScale() const { return m_Transformable.getScale(); }

		sf::Vector2f GetOrigin() const { return m_Transformable.getOrigin(); }

		const sf::Transform& GetTransform() const { return m_Transformable.getTransform(); }

		const sf::Transform& GetInverseTransform() const { return m_Transformable.getInverseTransform(); }

		T& Move(sf::Vector2f offset);

		T& Move(float x, float y);

		T& Rotate(Degrees angle);

		T& Scale(sf::Vector2f factor);

		T& Scale(float x, float y);

	protected:
		sf::Transformable& m_Transformable;
		bool m_TransformChanged = false;
	private:
		T& m_Caller;
	};
}

#include "Components/Transform2DWrapper.inl"