#include "Transform2DWrapper.h"

namespace Smasher {
	template <class T>
	T& Transform2DWrapper<T>::SetPosition(sf::Vector2f position) {
		m_TransformChanged = true;
		m_Transformable.setPosition(position);
		return m_Caller;
	}

	template <class T>
	T& Transform2DWrapper<T>::SetPosition(float x, float y) {
		m_TransformChanged = true;
		m_Transformable.setPosition({ x, y });
		return m_Caller;
	}

	template <class T>
	T& Transform2DWrapper<T>::SetRotation(Degrees angle) {
		m_TransformChanged = true;
		m_Transformable.setRotation(sf::degrees(angle));
		return m_Caller;
	}

	template <class T>
	T& Transform2DWrapper<T>::SetScale(sf::Vector2f factors) {
		m_TransformChanged = true;
		m_Transformable.setScale(factors);
		return m_Caller;
	}

	template <class T>
	T& Transform2DWrapper<T>::SetScale(float x, float y) {
		m_TransformChanged = true;
		m_Transformable.setScale({ x, y });
		return m_Caller;
	}

	template <class T>
	T& Transform2DWrapper<T>::SetOrigin(float x, float y) {
		m_TransformChanged = true;
		m_Transformable.setOrigin({ x, y });
		return m_Caller;
	}

	template <class T>
	T& Transform2DWrapper<T>::SetOrigin(sf::Vector2f origin) {
		m_TransformChanged = true;
		m_Transformable.setOrigin(origin);
		return m_Caller;
	}

	template <class T>
	T& Transform2DWrapper<T>::Move(sf::Vector2f offset) {
		m_TransformChanged = true;
		m_Transformable.move(offset);
		return m_Caller;
	}

	template <class T>
	T& Transform2DWrapper<T>::Move(float x, float y) {
		m_TransformChanged = true;
		m_Transformable.move({ x, y });
		return m_Caller;
	}

	template <class T>
	T& Transform2DWrapper<T>::Rotate(Degrees angle) {
		m_TransformChanged = true;
		m_Transformable.rotate(sf::degrees(angle));
		return m_Caller;
	}

	template <class T>
	T& Transform2DWrapper<T>::Scale(sf::Vector2f factor) {
		m_TransformChanged = true;
		m_Transformable.scale(factor);
		return m_Caller;
	}

	template <class T>
	T& Transform2DWrapper<T>::Scale(float x, float y) {
		m_TransformChanged = true;
		m_Transformable.scale({ x, y });
		return m_Caller;
	}
}