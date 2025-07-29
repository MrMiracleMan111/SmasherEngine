#pragma once
#include <unordered_map>
#include <cassert>
#include <filesystem>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Base.h"
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
		ResourceHandle(ResourceID id, const ResourcePath& path, ResourceType type) :
			m_ID(id), m_Path(&path), m_Type(type) {}
		~ResourceHandle() = default;
		ResourceHandle(const ResourceHandle&) = default;
		ResourceHandle(ResourceHandle&& other) noexcept :
			m_ID(other.m_ID),
			m_Path(other.m_Path),
			m_Type(other.m_Type) {};
		ResourceHandle& operator=(const ResourceHandle&) = default;
		ResourceHandle& operator=(ResourceHandle&& other) noexcept {
			if (&other != this) {
				m_ID = other.m_ID;	   // Copy value over
				m_Path = other.m_Path; // Copy value over
				m_Type = other.m_Type; // Copy value over
			}
			return *this;
		};

		ResourceID GetID() const { return m_ID; }
		ResourcePath const* GetPath() const { return m_Path; }
		ResourceType GetType() const { return m_Type; }

	private:
		ResourceID m_ID = UINT64_MAX;
		ResourcePath const * m_Path = nullptr;
		ResourceType m_Type = ResourceType::INVALID;
	};

	class SMASHER_API ResourceManager final {
	public:
		template <class T>
		ResourceHandle GetResourceHandle(ResourceType type) {
			static_assert(ManifestItemHasResourceID<T>, "Manifest item must have an ID");
			static_assert(ManifestItemHasResourcePath<T>, "Manifest item must have an PATH");

			static_assert(std::is_class_v<T>, "Manifest item T must be a struct or class");
			static_assert(std::is_same_v<typename std::decay<decltype(T::ID)>::type, ResourceID>, "Manifest item ID must be of type ResourceID");
			static_assert(std::is_same_v<typename std::decay<decltype(T::PATH)>::type, ResourcePath>, "Manifest item PATH must be of type ResourcePath");

			return ResourceHandle(T::ID, T::PATH, type);
		}

		// Lazily loads resources
		template <class T>
		std::shared_ptr<T> GetResource(const ResourceHandle& handle) {
			static_assert(std::is_base_of<Resource, T>::value, "T must inherit from Resource");
			if (handle.GetType() != T::GetStaticType()) {
				throw Exceptions::ResourceTypeMismatch(std::format("{} ResourceType doesn't match handle's ResourceType", typeid(T).name()));
			}

			if (m_ResourceMap.find(handle.GetID()) == m_ResourceMap.end()) {
				return std::static_pointer_cast<T>(LoadResource<T>(handle.GetID(), *handle.GetPath()));
			}
			return std::static_pointer_cast<T>(m_ResourceMap.at(handle.GetID()));
		}

		// Remove internal internally, resource object will be deleted once all objects using it free it
		void ReleaseResource(ResourceID resourceID) {};

		void SetResourceDirectory(std::filesystem::path path) {
			m_ResourceDirectory = path;
		}

		const std::filesystem::path& GetResourceDirectory() { return m_ResourceDirectory; }
	private:
		template <class T>
		std::shared_ptr<T> LoadResource(ResourceID resourceID, const ResourcePath& path) {
			std::shared_ptr<T> pResource = std::make_shared<T>(resourceID, path, m_ResourceDirectory);
			std::shared_ptr<Resource> pCastResource = std::static_pointer_cast<Resource>(pResource);
			m_ResourceMap.emplace(resourceID, pCastResource);
			return pResource;
		}

		std::unordered_map<ResourceID, std::shared_ptr<Resource>> m_ResourceMap;
		std::filesystem::path m_ResourceDirectory;
	};
}