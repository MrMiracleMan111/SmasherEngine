#pragma once
#include "Smasher/Base.h"
#include <memory>
namespace Smasher {
	class UUID;
}

class SMASHER_API Smasher::UUID {
public:
	static UUID GetUUID();
	~UUID() {};
	UUID(const UUID &other) : m_Uuid(other.m_Uuid) {};
	UUID(UUID &&other) noexcept;
	UUID& operator = (const UUID &other);
	UUID& operator = (UUID &&other) noexcept;
	explicit UUID(uint64_t uuid) : m_Uuid(uuid) {};
	UUID() = delete;

	operator uint64_t() const { return m_Uuid; }
	bool operator == (const UUID& other) const { return other.m_Uuid == m_Uuid; }
private:
	inline static uint64_t s_count = 0;
	uint64_t m_Uuid;
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