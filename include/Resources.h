#pragma once
#include "Base.h"
#include "Resources.h"

#define SMASHER_RESOURCE_TYPE(type) const ResourceType& GetType() const { return GetStaticResourceType();} \
static ResourceType GetStaticResourceType() { return type; } \


namespace Smasher {
	class SMASHER_API Resource {
		friend class ResourceManager;
	public:
		Resource() = delete;
		virtual ~Resource() {};
		virtual const ResourceType& GetType() = 0;
	protected:
		Resource(ResourceID id, ResourcePath path) : m_ID(id), m_Path(path) {}

	private:
		bool m_Loaded = false;
		ResourceID m_ID;
		ResourcePath& m_Path;
	};


	class SMASHER_API TextureResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::TEXTURE)
		TextureResource() = delete;
		TextureResource(ResourceID id, ResourcePath path) : Resource(id, path) {
			if (!m_Texture.loadFromFile(path.string())) {
				throw Exceptions::ResourceFailedToLoad(std::format("Failed to load file {}", path.string()));
			}
		}
		sf::Texture& GetTexture() { return m_Texture; }
	private:
		sf::Texture m_Texture;
	};
	class SMASHER_API FontResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::FONT)
			FontResource() = delete;
		FontResource(ResourceID id, ResourcePath path) : Resource(id, path) {
			if (!m_Font.loadFromFile(path.string())) {
				throw Exceptions::ResourceFailedToLoad(std::format("Failed to load file {}", path.string()));
			}
		}
	private:
		sf::Font m_Font;
	};
	class SMASHER_API AudioResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::AUDIO)
			AudioResource() = delete;
		AudioResource(ResourceID id, ResourcePath path) : Resource(id, path) {
			if (!m_Music.openFromFile(path.string())) {
				throw Exceptions::ResourceFailedToLoad(std::format("Failed to load file {}", path.string()));
			}
		}
	private:
		sf::Music m_Music;
	};
	class SMASHER_API ShaderResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::SHADER)
	};

}