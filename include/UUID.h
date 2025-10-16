#pragma once
#include "Base.h"
#include <memory>
namespace Smasher {
	class UUID;
}

class SMASHER_API Smasher::UUID {
public:
	static UUID GetUUID();
	~UUID() {};
	UUID(const UUID& other) : m_UUID(other.m_UUID) {};
	UUID(UUID&& other) noexcept;
	UUID& operator = (const UUID& other) = default;
	UUID& operator = (UUID&& other) noexcept;
	explicit UUID(uint64_t uuid) : m_UUID(uuid) {};
	UUID() = delete;

	operator uint64_t() const { return m_UUID; }
	bool operator == (const UUID& other) const { return other.m_UUID == m_UUID; }
private:
	inline static uint64_t s_count = 0;
	uint64_t m_UUID;
};

namespace std {
	template<>
	struct hash<Smasher::UUID>
	{
		std::size_t operator()(const Smasher::UUID& uuid) const noexcept
		{
			return hash<uint64_t>{}((uint64_t)uuid);
		}
	};
}