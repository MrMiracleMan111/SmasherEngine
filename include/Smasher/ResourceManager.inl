#pragma once

namespace Smasher {
	// Lazily Loads Resource
	template <class ManifestData, class T, typename... Args>
	std::shared_ptr<T> ResourceManager::GetOrLoadResource(Args&&... resourceArgs) {
		static_assert(ManifestItemHasResourceId<ManifestData>, "Manifest item must have an Id");
		static_assert(std::is_class_v<ManifestData>, "Manifest item T must be a struct or class");
		static_assert(std::is_same_v<typename std::decay<decltype(ManifestData::Id)>::type, ResourceId>, "Manifest item Id must be of type ResourceId");
		static_assert(std::is_base_of<Resource, T>::value, "Resource type must inherit from Resource");

		// static_assert(HasPathOrPathsVariable<ManifestData>, "Manifest item must either have PATH of type ResourcePath or PATHS of type ResourcePath*"); 

		if constexpr (HasPathsVariable<ManifestData>) {
			constexpr std::size_t numPaths = sizeof(ManifestData::PATHS) / sizeof(ResourcePath);
			return LoadResource<T>(ManifestData::Id, ManifestData::PATHS, numPaths, resourceArgs...);
		}
		else if constexpr (HasPathVariable<ManifestData>) {
			return LoadResource<T>(ManifestData::Id, &ManifestData::PATH, 1, resourceArgs...);
		}

		assert(false);
		return std::shared_ptr<T>(nullptr);
	}

	// Retrieves resources
	template <class ManifestData, class T>
	std::shared_ptr<T> ResourceManager::GetResource() {
		static_assert(ManifestItemHasResourceId<ManifestData>, "Manifest item must have an ID");
		static_assert(ManifestItemHasResourcePath<ManifestData>, "Manifest item must have an PATH");
		static_assert(std::is_class_v<ManifestData>, "Manifest item T must be a struct or class");
		static_assert(std::is_same_v<typename std::decay<decltype(ManifestData::Id)>::type, ResourceId>, "Manifest item ID must be of type ResourceID");
		static_assert(std::is_same_v<typename std::decay<decltype(ManifestData::PATH)>::type, ResourcePath> ||
			std::is_same_v<typename std::decay<decltype(ManifestData::PATHS)>::type, ResourcePath*>, "Manifest item must either have PATH of type ResourcePath or PATHS of type ResourcePath*");
		static_assert(std::is_base_of<Resource, T>::value, "U must inherit from Resource");

		if (m_ResourceMap.find(ManifestData::Id) == m_ResourceMap.end()) {
			throw Exceptions::ResourceNotLoaded(std::format("Resource {} not loaded", ManifestData::PATH.string()));
		}

		return std::static_pointer_cast<T>(m_ResourceMap.at(ManifestData::Id));
	}

	template <class T>
	std::shared_ptr<T> ResourceManager::GetResource(ResourceId resourceId) {
		static_assert(std::is_base_of<Resource, T>::value, "T must inherit from Resource");

		if (m_ResourceMap.find(resourceId) == m_ResourceMap.end()) {
			throw Exceptions::ResourceNotLoaded(std::format("Resource with ID {} not loaded", resourceId));
		}
		return std::static_pointer_cast<T>(m_ResourceMap.at(resourceId));
	}

	// Retrieves resources
	template <class ManifestData>
	Expected<ResourceId> ResourceManager::GetResourceId() {
		static_assert(ManifestItemHasResourceId<ManifestData>, "Manifest item must have an ID");
		static_assert(ManifestItemHasResourcePath<ManifestData>, "Manifest item must have an PATH");
		static_assert(std::is_class_v<ManifestData>, "Manifest item T must be a struct or class");
		static_assert(std::is_same_v<typename std::decay<decltype(ManifestData::Id)>::type, ResourceId>, "Manifest item ID must be of type ResourceID");
		static_assert(std::is_same_v<typename std::decay<decltype(ManifestData::PATH)>::type, ResourcePath> ||
			std::is_same_v<typename std::decay<decltype(ManifestData::PATHS)>::type, ResourcePath*>, "Manifest item must either have PATH of type ResourcePath or PATHS of type ResourcePath*");

		if (m_ResourceMap.find(ManifestData::Id) == m_ResourceMap.end()) {
			return Expected<ResourceId>::Error(ERROR_ResourceNotLoaded);
		}

		return ManifestData::Id;
	}

	// Retrieves resources Path Information
	template <class ManifestData>
	Expected<ResourceManifestInfo> ResourceManager::GetManifestInfo() {
		static_assert(ManifestItemHasResourceId<ManifestData>, "Manifest item must have an ID");
		static_assert(ManifestItemHasResourcePath<ManifestData>, "Manifest item must have an PATH");
		static_assert(std::is_class_v<ManifestData>, "Manifest item T must be a struct or class");
		static_assert(std::is_same_v<typename std::decay<decltype(ManifestData::Id)>::type, ResourceId>, "Manifest item ID must be of type ResourceID");
		static_assert(std::is_same_v<typename std::decay<decltype(ManifestData::PATH)>::type, ResourcePath> ||
			std::is_same_v<typename std::decay<decltype(ManifestData::PATHS)>::type, ResourcePath*>, "Manifest item must either have PATH of type ResourcePath or PATHS of type ResourcePath*");

		if constexpr (HasPathsVariable<ManifestData>) {
			constexpr std::size_t numPaths = sizeof(ManifestData::PATHS) / sizeof(ResourcePath);
			return { ManifestData::Id, ManifestData::PATHS, numPaths };
		}
		else if constexpr (HasPathVariable<ManifestData>) {
			return { ManifestData::Id, &ManifestData::PATH, 1 };
		}

		// Should be unreachable
		static_assert(false, "Missing PATH or PATHS");
		return {};
	}

	template <class T, typename... Args>
	std::shared_ptr<T> ResourceManager::LoadResource(ResourceId resourceId, const ResourcePath *paths, const std::size_t numPaths, Args&&... resourceArgs) {
		auto itr = m_ResourceMap.find(resourceId);
		if (itr != m_ResourceMap.end()) {
			return std::static_pointer_cast<T>(itr->second);
		}
		std::shared_ptr<T> pResource = std::make_shared<T>(resourceId, paths, numPaths, m_ResourceDirectory, std::forward<Args>(resourceArgs)...);
		std::shared_ptr<Resource> pCastResource = std::static_pointer_cast<Resource>(pResource);
		m_ResourceMap.emplace(resourceId, pCastResource);
		return pResource;
	}
}