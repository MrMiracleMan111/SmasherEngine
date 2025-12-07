#include "Smasher/UUID.h"

namespace Smasher {
	UUID UUID::GetUUID() {
		++s_count;
		return UUID{ s_count };
	}

	UUID::UUID(UUID &&other) noexcept : m_Uuid(other.m_Uuid)
	{
		other.m_Uuid = UINT64_MAX;
	}

	UUID& UUID::operator=(const UUID &other)
	{
		m_Uuid = other.m_Uuid;
		return *this;
	}

	UUID& UUID::operator=(UUID &&other) noexcept
	{
		if (&other != this) {
			m_Uuid = other.m_Uuid;
			other.m_Uuid = UINT64_MAX;
		}
		return *this;
	}
}