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
		Resource(ResourceID id, 
			const ResourcePath* const relativePaths, const size_t numPaths,
			const ResourcePath& resourcesDirectory) :
			m_ID(id), m_Paths(numPaths, "") {

			assert(m_Paths.size() > 0);

			// Using std::min just in case there is some wacky issue
			for (size_t i = 0; i < std::min(numPaths, m_Paths.size()); i++) {
				ResourcePath& path = m_Paths[i];
				path += resourcesDirectory;
				path += relativePaths[i];
			}
		}

	protected:
		bool m_Loaded = false;
		ResourceID m_ID;
		std::vector<ResourcePath> m_Paths;
	};


	class SMASHER_API TextureResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::TEXTURE)
		TextureResource() = delete;
		TextureResource(ResourceID id,
			const ResourcePath* const relativePaths, const size_t numPaths,
			const ResourcePath& resourcesDirectory) :
			Resource(id, relativePaths, numPaths, resourcesDirectory) {
			assert(numPaths > 0);

			if (!m_Texture.loadFromFile(m_Paths[0].string())) {
				throw Exceptions::ResourceFailedToLoad(std::format("Failed to load file {}", m_Paths[0].string()));
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
		FontResource(ResourceID id,
			const ResourcePath* const relativePaths, const size_t numPaths,
			const ResourcePath& resourcesDirectory) :
			Resource(id, relativePaths, numPaths, resourcesDirectory) {
			if (!m_Font.loadFromFile(m_Paths[0].string())) {
				throw Exceptions::ResourceFailedToLoad(std::format("Failed to load file {}", m_Paths[0].string()));
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
		AudioResource(ResourceID id,
			const ResourcePath* const relativePaths, const size_t numPaths,
			const ResourcePath& resourcesDirectory) :
			Resource(id, relativePaths, numPaths, resourcesDirectory) {
			if (!m_Music.openFromFile(m_Paths[0].string())) {
				throw Exceptions::ResourceFailedToLoad(std::format("Failed to load file {}", m_Paths[0].string()));
			}
		}
	private:
		sf::Music m_Music;
	};

	class SMASHER_API ShaderResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::SHADER)
			ShaderResource() = delete;

		// One Shader File - Type must be specified
		ShaderResource(ResourceID id,
			const ResourcePath* const relativePaths, const size_t numPaths,
			const ResourcePath& resourcesDirectory, sf::Shader::Type type) :
			Resource(id, relativePaths, numPaths, resourcesDirectory) {

			assert(numPaths == 1);
			m_Shader.loadFromFile(m_Paths[0].string(), type);
		}

		ShaderResource(ResourceID id,
			const ResourcePath* const relativePaths, const size_t numPaths,
			const ResourcePath& resourcesDirectory) :
			Resource(id, relativePaths, numPaths, resourcesDirectory) {
			
			switch (numPaths) {
			case 2:
				m_Shader.loadFromFile(m_Paths[0].string(), m_Paths[1].string());
				break;
			case 3:
				m_Shader.loadFromFile(
					m_Paths[0].string(),
					m_Paths[1].string(),
					m_Paths[2].string());
			default:
				assert(false); // Never should be reached
			}
		}

		sf::Shader& GetShader() { return m_Shader; }

	private:
		sf::Shader m_Shader;
		sf::Shader::Type m_ShaderType;
	};

	class SMASHER_API FileResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::FILE)
		FileResource() = delete;
		FileResource(ResourceID id,
			const ResourcePath* const relativePaths, const size_t numPaths,
			const ResourcePath& resourcesDirectory, std::ios_base::openmode mode) :
			Resource(id, relativePaths, numPaths, resourcesDirectory), m_File(m_Paths[0], mode) {

		}
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