#pragma once
#include <ios>
#include <fstream>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Audio.hpp>

#include "Smasher/Base.h"

#define SMASHER_RESOURCE_TYPE(type) ResourceType GetType() const { return GetStaticType();} \
static constexpr ResourceType GetStaticType() { return type; } \


namespace Smasher {
	struct SMASHER_API ResourceManifestInfo {
		const ResourcePath* paths;
		const std::size_t path_count;
	};

	class SMASHER_API Resource {
		friend class ResourceManager;
	public:
		Resource() = delete;
		const ResourceId GetId() const { return m_Id; }
		virtual ~Resource() {};
		virtual ResourceType GetType() const = 0;
	protected:
		Resource(ResourceId id, 
			const ResourcePath *const relativePaths, const std::size_t numPaths,
			const ResourcePath &resourcesDirectory) :
			m_Id(id), m_Paths(numPaths, "") {

			assert(m_Paths.size() > 0);

			// Using std::min just in case there is some wacky issue
			for (std::size_t i = 0; i < std::min(numPaths, m_Paths.size()); i++) {
				m_Paths[i] += resourcesDirectory;
				m_Paths[i] += relativePaths[i];
			}
		}

		// Constructor for resources with no paths (ex. shaders generated from std::string)
		Resource(ResourceId id) : m_Id(id), m_Paths(), m_Loaded(true) {}

	protected:
		bool m_Loaded = false;
		ResourceId m_Id;
		std::vector<ResourcePath> m_Paths;
	};


	class SMASHER_API TextureResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::TEXTURE)
		TextureResource() = delete;
		TextureResource(ResourceId id,
			const ResourcePath *const relativePaths, const std::size_t numPaths,
			const ResourcePath &resourcesDirectory) :
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
		FontResource(ResourceId id,
			const ResourcePath *const relativePaths, const std::size_t numPaths,
			const ResourcePath &resourcesDirectory) :
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
		AudioResource(ResourceId id,
			const ResourcePath *const relativePaths, const std::size_t numPaths,
			const ResourcePath &resourcesDirectory) :
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

		ShaderResource(ResourceId id,
			const ResourcePath *const relativePaths, const std::size_t numPaths,
			const ResourcePath &resourcesDirectory) :
			Resource(id, relativePaths, numPaths, resourcesDirectory),
			m_ShaderType{sf::Shader::Type::Fragment}  {
			
			switch (numPaths) {
			case 1:
				m_Shader.loadFromFile(m_Paths[0].string(), sf::Shader::Type::Fragment);
				break;
			case 2:
				m_Shader.loadFromFile(m_Paths[0].string(), m_Paths[1].string());
				break;
			case 3:
				m_Shader.loadFromFile(
					m_Paths[0].string(),
					m_Paths[1].string(),
					m_Paths[2].string());
				break;
			default:
				throw Smasher::Exceptions::ResourceInvalidNumPaths("numPaths is invalid");
				assert(false); // Never should be reached
			}
		}

		ShaderResource(ResourceId id, const std::string &vert, const std::string &frag) :
			Resource(id), m_ShaderType(sf::Shader::Type::Geometry) {
			m_Shader.loadFromMemory(vert, frag);
		}

		sf::Shader& GetShader() { return m_Shader; }

	private:
		sf::Shader m_Shader;
		sf::Shader::Type m_ShaderType = sf::Shader::Type::Fragment;
	};

	class SMASHER_API FileResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::FILE)
		FileResource() = delete;
		FileResource(ResourceId id,
			const ResourcePath *const relativePaths, const std::size_t numPaths,
			const ResourcePath &resourcesDirectory, std::ios_base::openmode mode) :
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