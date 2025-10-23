#pragma once
#include <list>
#include <memory.h>
#include <unordered_map>
#include <typeindex>
#include <format>
#include <SFML/Window.hpp>
#include "Smasher/Base.h"
#include "Smasher/UUID.h"
#include "Smasher/Layer.h"

namespace Smasher {

	class IComponent;
	class Engine;
	class Layer;

	class SMASHER_API Entity {
		friend class Layer;
	public:
		Entity() = delete;
		Entity(Layer& state, UUID uuid) : m_LayerRef(state), m_UUID(uuid), m_Engine(state.GetEngine()) {};
		Entity(const Entity& other) = delete;
		Entity(Entity&& other) noexcept : m_LayerRef(other.m_LayerRef), m_UUID(other.m_UUID), m_Engine(other.m_Engine) {};
		Entity& operator =(const Entity& other) = delete;
		Entity& operator =(Entity&&) = delete; // No need for this so far and seems dangerous
		virtual ~Entity();

		Layer& GetLayer() { return m_LayerRef.get(); };
		Engine& GetEngine() { return m_Engine; };
		UUID GetUUID() const { return m_UUID; };

		virtual void Init() {}

		template<class T>
		bool HasComponent() {
			const std::type_index index = std::type_index(typeid(T));
			return m_ComponentsByType.find(index) != m_ComponentsByType.end();
		}

		template<class T>
		void DependsOnComponent() {
			if (!HasComponent<T>()) {
				throw Exceptions::MissingComponentDependency(std::format("Missing Component Dependency {}", typeid(T).name()));
			}
		}

		template<IComponentType T, typename... Args>
		T& AddComponent(Args&&... componentArgs);

		template<class T>
		void RemoveComponent();

		template<class T>
		T& GetComponent() const;

	protected:
		void SetLayer(Layer& layer) { m_LayerRef = layer; }

	private:
		const UUID m_UUID;
		std::reference_wrapper<Layer> m_LayerRef;
		Engine& m_Engine;
		std::unordered_map<std::type_index, std::reference_wrapper<IComponent>> m_ComponentsByType;
	};
}

#include "Entity.inl"