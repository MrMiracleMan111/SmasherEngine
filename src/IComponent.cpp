#include "Base.h"
#include "IComponent.h"

namespace Smasher {
	IComponent& IComponent::operator = (IComponent&& other) noexcept {
		if (this != &other) {
			m_Entity = other.m_Entity;
			m_Status = other.m_Status;
			m_Manager = other.m_Manager;
			other.m_Status = ComponentStatus::INVALID;
		}
		return *this;
	}

	IComponent::~IComponent() {
		m_Entity = nullptr;
		m_Manager = nullptr;
		m_Status = ComponentStatus::INVALID;
	};
}