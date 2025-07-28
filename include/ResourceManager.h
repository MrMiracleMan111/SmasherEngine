#pragma once
#include <unordered_map>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Base.h"
#include "ResourceUtils.h"
#include "Resources.h"

/**

Design Criteria

Thread Safe Resource Loading and Retrieval

*/

namespace Smasher {
	struct SMASHER_API ResourceHandle {
	public:
		ResourceHandle() : 
			m_ID(UINT64_MAX), m_Path(nullptr), m_Type(ResourceType::INVALID) {}
		ResourceHandle(ResourceID id, ResourcePath& path, ResourceType type) :
			m_ID(id), m_Path(&path), m_Type(type) {}

		ResourceID GetID() const { return m_ID; }
		ResourcePath* GetPath() const { return m_Path; }
		ResourceType GetType() const { return m_Type; }

	private:
		ResourceID m_ID = UINT64_MAX;
		ResourcePath* const m_Path = nullptr;
		ResourceType m_Type = ResourceType::INVALID;
	};

	class SMASHER_API ResourceManager final {
	public:
		template <class T>
		ResourceHandle GetResourceHandle(ResourceType type) {
			static_assert(std::is_class_v<T>, "Manifest item T must be a struct or class");
			static_assert(std::is_same_v<T::ID, ResourceID>, "Manifest item ID must be of type ResourceID");
			static_assert(std::is_same_v<T::Path, ResourcePath>, "Manifest item PATH must be of type ResourcePath");

			return { T::ID, T::PATH, type };
		}

		// Lazily loads resources
		template <class T>
		std::shared_ptr<T> GetResource(const ResourceHandle& handle) {
			static_assert(std::is_base_of<Resource, T>::value, "T must inherit from Resource");
			static_assert(handle.GetType() == T::GetStaticType(), "T ResourceType must match handle's ResourceType");

			if (m_ResourceMap.find(handle.m_ID) == m_ResourceMap.end()) {
				return std::static_pointer_cast<T>(LoadResource<T>(handle.m_ID, *handle.m_Path));
			}
			return std::static_pointer_cast<T>(m_ResourceMap.at(handle.m_ID));
		}

		// Remove internal internally, resource object will be deleted once all objects using it free it
		void ReleaseResource(ResourceID resourceID) {};

	private:
		template <class T>
		std::shared_ptr<Resource> LoadResource(ResourceID resourceID, ResourcePath path) {
			m_ResourceMap.emplace(resourceID, resourceID, path);
		}

		std::unordered_map<ResourceID, std::shared_ptr<Resource>> m_ResourceMap;
	};
}