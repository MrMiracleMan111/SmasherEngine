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
	class SMASHER_API ResourceManager final {
	public:
		// Lazily Loads Resource
		template <class ManifestData, class T, typename... Args>
		std::shared_ptr<T> GetOrLoadResource(Args&&... resourceArgs) {
			static_assert(ManifestItemHasResourceID<ManifestData>, "Manifest item must have an ID");
			static_assert(ManifestItemHasResourcePath<ManifestData>, "Manifest item must have an PATH");
			static_assert(std::is_class_v<ManifestData>, "Manifest item T must be a struct or class");
			static_assert(std::is_same_v<typename std::decay<decltype(ManifestData::ID)>::type, ResourceID>, "Manifest item ID must be of type ResourceID");
			static_assert(std::is_same_v<typename std::decay<decltype(ManifestData::PATH)>::type, ResourcePath>, "Manifest item PATH must be of type ResourcePath");
			static_assert(std::is_base_of<Resource, T>::value, "U must inherit from Resource");

			return LoadResource<T>(ManifestData::ID, ManifestData::PATH, resourceArgs...);
		}

		// Lazily Loads Resource
		template <class T, typename... Args>
		std::shared_ptr<T> GetOrLoadResource(ResourceID id, ResourcePath path, Args&&... resourceArgs) {
			return LoadResource<T>(id, path, resourceArgs...);
		}


		// Retrieves resources
		template <class ManifestData, class T>
		std::shared_ptr<T> GetResource() {
			static_assert(ManifestItemHasResourceID<ManifestData>, "Manifest item must have an ID");
			static_assert(ManifestItemHasResourcePath<ManifestData>, "Manifest item must have an PATH");
			static_assert(std::is_class_v<ManifestData>, "Manifest item T must be a struct or class");
			static_assert(std::is_same_v<typename std::decay<decltype(ManifestData::ID)>::type, ResourceID>, "Manifest item ID must be of type ResourceID");
			static_assert(std::is_same_v<typename std::decay<decltype(ManifestData::PATH)>::type, ResourcePath>, "Manifest item PATH must be of type ResourcePath");
			static_assert(std::is_base_of<Resource, T>::value, "U must inherit from Resource");

			if (m_ResourceMap.find(ManifestData::ID) == m_ResourceMap.end()) {
				throw Exceptions::ResourceNotLoaded(std::format("Resource {} not loaded", ManifestData::PATH.string()));
			}

			return std::static_pointer_cast<T>(m_ResourceMap.at(ManifestData::ID));
		}


		template <class T>
		std::shared_ptr<T> GetResource(ResourceID resourceID, ResourcePath& path) {
			static_assert(std::is_base_of<Resource, T>::value, "T must inherit from Resource");

			if (m_ResourceMap.find(resourceID) == m_ResourceMap.end()) {
				throw Exceptions::ResourceNotLoaded(std::format("Resource {} not loaded", path.string()));
			}
			return std::static_pointer_cast<T>(m_ResourceMap.at(path));
		}

		// Remove internal internally, resource object will be deleted once all objects using it free it
		void ReleaseResource(ResourceID resourceID) {
			m_ResourceMap.erase(resourceID);
		};

		void SetResourceDirectory(std::filesystem::path path) {
			m_ResourceDirectory = path;
		}

		const std::filesystem::path& GetResourceDirectory() { return m_ResourceDirectory; }
	private:
		template <class T, typename... Args>
		std::shared_ptr<T> LoadResource(ResourceID resourceID, const ResourcePath& path, Args&&... resourceArgs) {
			auto itr = m_ResourceMap.find(resourceID);
			if (itr != m_ResourceMap.end()) {
				return std::static_pointer_cast<T>(itr->second);
			}

			std::shared_ptr<T> pResource = std::make_shared<T>(resourceID, path, m_ResourceDirectory, std::forward<Args>(resourceArgs)...);
			std::shared_ptr<Resource> pCastResource = std::static_pointer_cast<Resource>(pResource);
			m_ResourceMap.emplace(resourceID, pCastResource);
			return pResource;
		}

		std::unordered_map<ResourceID, std::shared_ptr<Resource>> m_ResourceMap;
		std::filesystem::path m_ResourceDirectory;
	};
}