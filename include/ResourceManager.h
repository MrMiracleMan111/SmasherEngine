#pragma once
#include <unordered_map>
#include "Base.h"
#include "ResourceUtils.h"

/**

Design Criteria

Thread Safe Resource Loading and Retrieval

*/

#define SMASHER_RESOURCE_TYPE(type) const ResourceType& GetType() const { return m_Type;} \
static ResourceType GetStaticResourceType() { return type; } \

namespace Smasher {

	enum class ResourceType {
		TEXTURE,
		FONT,
		AUDIO,
		SHADER
	};

	class Resource {
		friend class ResourceManager;
	public:
		Resource() = delete;

		const ResourceType& GetType() const {
			return m_Type;
		}
	protected:
		Resource(ResourceType type) : m_Type(type) {}
		const ResourceType m_Type;

	private:
		bool m_Loaded = false;
	};

	class TextureResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::TEXTURE)
	};
	class FontResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::FONT)
	};
	class AudioResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::AUDIO)
	};
	class ShaderResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::SHADER)
	};

	struct ResourceHandle {
		const ResourceID& ID;
		const ResourcePath& Path;
	};

	class ResourceManager {
	public:
		template <class T>
		ResourceHandle GetResourceHandle() {
			static_assert(std::is_class_v<T>, "Manifest item T must be a struct or class")
			static_assert(std::is_same_v<T::ID, ResourceID>, "Manifest item ID must be of type ResourceID");
			static_assert(std::is_same_v<T::Path, ResourcePath>, "Manifest item PATH must be of type ResourcePath");

			return { T::ID, T::PATH }
		}

		// Lazily loads resources
		template <class T>
		std::shared_ptr<T> GetResource(const ResourceHandle& handle) {
			static_assert(std::is_base_of<Resource, T>::value, "T must inherit from Resource");
			static_assert(handle.GetType() == T::GetStaticType(), "T ResourceType must match handle's ResourceType");

			if (m_ResourceMap.find(handle.ID) == m_ResourceMap.end()) {
				return std::static_pointer_cast<T>(LoadResource<T>(handle.ID, handle.Path));
			}
			return std::static_pointer_cast<T>(m_ResourceMap.at(handle.ID));
		}

		// Remove internal internally, resource object will be deleted once all objects using it free it
		void ReleaseResource(ResourceID resourceID);

	private:
		template <class T>
		std::shared_ptr<Resource> LoadResource(ResourceID resourceID, ResourcePath path) {
			m_ResourceMap.emplace(resourceID, resourceID, path);
		}

		std::unordered_map<ResourceID, std::shared_ptr<Resource>> m_ResourceMap;
	};
}