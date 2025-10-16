#include "UUID.h"

namespace Smasher {
	UUID UUID::GetUUID() {
		++s_count;
		return UUID{ s_count };
	}

	UUID::UUID(UUID&& other) noexcept : m_UUID(other.m_UUID)
	{
		other.m_UUID = UINT64_MAX;
	}

	UUID& UUID::operator=(UUID&& other) noexcept
	{
		if (&other != this) {
			m_UUID = other.m_UUID;
			other.m_UUID = UINT64_MAX;
		}
		return *this;
	}
}