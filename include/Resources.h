#pragma once
#include <ios>
#include <fstream>
#include "Base.h"

#define SMASHER_RESOURCE_TYPE(type) ResourceType GetType() const { return GetStaticType();} \
static constexpr ResourceType GetStaticType() { return type; } \


namespace Smasher {
	class SMASHER_API Resource {
		friend class ResourceManager;
	public:
		Resource() = delete;
		virtual ~Resource() {};
		virtual ResourceType GetType() const = 0;
	protected:
		Resource(ResourceID id, const ResourcePath& relativePath, const std::filesystem::path& resourcesDirectory) :
			m_ID(id), m_Path(resourcesDirectory) {
			m_Path += relativePath;
		}

	protected:
		bool m_Loaded = false;
		ResourceID m_ID;
		ResourcePath m_Path;
	};


	class SMASHER_API TextureResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::TEXTURE)
		TextureResource() = delete;
		TextureResource(ResourceID id, const ResourcePath& relativePath, const std::filesystem::path& resourcesDirectory) :
			Resource(id, relativePath, resourcesDirectory) {
			if (!m_Texture.loadFromFile(m_Path.string())) {
				throw Exceptions::ResourceFailedToLoad(std::format("Failed to load file {}", m_Path.string()));
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
		FontResource(ResourceID id, const ResourcePath& relativePath, const std::filesystem::path& resourcesDirectory) :
			Resource(id, relativePath, resourcesDirectory) {
			if (!m_Font.loadFromFile(m_Path.string())) {
				throw Exceptions::ResourceFailedToLoad(std::format("Failed to load file {}", m_Path.string()));
			}
		}

		sf::Font& GetFont() { return m_Font; }
	private:
		sf::Font m_Font;
	};
	class SMASHER_API AudioResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::AUDIO)
			AudioResource() = delete;
		AudioResource(ResourceID id, const ResourcePath& relativePath, const std::filesystem::path& resourcesDirectory) :
			Resource(id, relativePath, resourcesDirectory) {
			if (!m_Music.openFromFile(m_Path.string())) {
				throw Exceptions::ResourceFailedToLoad(std::format("Failed to load file {}", m_Path.string()));
			}
		}
	private:
		sf::Music m_Music;
	};

	class SMASHER_API ShaderResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::SHADER)
			ShaderResource() = delete;
		ShaderResource(ResourceID id, const ResourcePath& relativePath, const std::filesystem::path& resourcesDirectory) :
			Resource(id, relativePath, resourcesDirectory) {}
	};

	class SMASHER_API FileResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::FILE)
		FileResource() = delete;
		FileResource(ResourceID id, const ResourcePath& relativePath, const std::filesystem::path& resourcesDirectory, std::ios_base::openmode mode) :
			Resource(id, relativePath, resourcesDirectory), m_File(m_Path, mode) {

		}
		FileResource(const FileResource& other) = default;
		FileResource(FileResource&& other) = default;
		~FileResource() {
			if (m_File.is_open()) {
				m_File.close();
			}
		}

		std::fstream& GetFileStream() { return m_File; }

	private:
		std::fstream m_File;
	};
}