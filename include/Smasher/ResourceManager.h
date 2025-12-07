#pragma once
#include <unordered_map>
#include <cassert>
#include <filesystem>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Smasher/Base.h"
#include "Smasher/Resources.h"

/**

Design Criteria

Thread Safe Resource Loading and Retrieval

*/

namespace Smasher {
	class SMASHER_API ResourceManager final {
	public:
		~ResourceManager() {};
		ResourceManager() = default;
		ResourceManager(ResourceManager&) = delete;
		ResourceManager(ResourceManager&&) = default;
		ResourceManager& operator=(ResourceManager&) = delete;
		ResourceManager& operator=(ResourceManager&&) = default;

		// Lazily Loads Resource
		template <class ManifestData, class T, typename... Args>
		std::shared_ptr<T> GetOrLoadResource(Args&&... resourceArgs);

		// Lazily Loads Resource that requires multiple paths
		template <class T, typename... Args>
		std::shared_ptr<T> GetOrLoadResource(ResourceId id, const ResourcePath* paths, const std::size_t numPaths, Args&&... resourceArgs) {
			return LoadResource<T>(id, paths, numPaths, resourceArgs...);
		}

		// Lazily Loads Resource from single path
		template <class T, typename... Args>
		std::shared_ptr<T> GetOrLoadResource(ResourceId id, const ResourcePath& path, Args&&... resourceArgs) {
			return GetOrLoadResource<T>(id, &path, size_t{ 1 }, resourceArgs...);
		}


		// Retrieves resources
		template <class ManifestData, class T>
		std::shared_ptr<T> GetResource();


		template <class T>
		std::shared_ptr<T> GetResource(ResourceId resourceID);

		// Generates shader from string
		std::shared_ptr<Smasher::ShaderResource> LoadVertFragShaderResource(const std::string& vert, const std::string& frag);

		// Remove internal internally, resource object will be deleted once all objects using it free it
		void ReleaseResource(ResourceId resourceID) {
			m_ResourceMap.erase(resourceID);
		};

		void SetResourceDirectory(std::filesystem::path path) {
			m_ResourceDirectory = path;
		}

		const std::filesystem::path& GetResourceDirectory() { return m_ResourceDirectory; }
	private:
		template <class T, typename... Args>
		std::shared_ptr<T> LoadResource(ResourceId resourceID, const ResourcePath* paths, const std::size_t numPaths, Args&&... resourceArgs);

		std::unordered_map<ResourceId, std::shared_ptr<Resource>> m_ResourceMap;
		std::filesystem::path m_ResourceDirectory;
	};
}

#include "ResourceManager.inl"