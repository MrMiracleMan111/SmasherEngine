#include "Smasher/Base.h"
#include "Smasher/IComponent.h"

namespace Smasher {
	IComponent& IComponent::operator = (IComponent &&other) noexcept {
		if (this != &other) {
			m_EntityPtr = other.m_EntityPtr;
			m_Status = other.m_Status;
			m_ManagerPtr = other.m_ManagerPtr;
			other.m_Status = ComponentStatus::INVALID;
		}
		return *this;
	}

	IComponent::~IComponent() {
		m_EntityPtr = nullptr;
		m_ManagerPtr = nullptr;
		m_Status = ComponentStatus::INVALID;
	};
}