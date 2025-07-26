#pragma once
#include "Base.h"
#include "Entity.h"

namespace Smasher {
	class ComponentManager;
	class Entity;

	class SMASHER_API Component {
		friend class ComponentManager;
	public:
		Component(Entity& entity) : m_Entity(entity), m_Index(SIZE_MAX), m_Valid(false) {};
		Component() = delete;
		virtual ~Component() = default;
		Component(Component&) = delete;
		Component(Component&& other) noexcept : m_Entity(other.m_Entity), m_Valid(other.m_Valid) { other.m_Valid = false; };
		Component& operator = (Component&) = delete;
		Component& operator = (Component&& other) noexcept {
			if (this != &other) {
				other.m_Valid = false;
			}
			return other;
		}
		bool IsValid() { return m_Valid; }
		Entity& GetEntity() { return m_Entity; }

	protected:
		void MakeValid() { m_Valid = true; }
		void SetIndex(size_t index) { m_Index = index; }
		size_t GetIndex() const { return m_Index; }

	private:
		Entity& m_Entity;
		size_t m_Index = SIZE_MAX; // Index component is located at within owning ComponentManager
		bool m_Valid = false; // Used by manager to prevent double remove
	};
}