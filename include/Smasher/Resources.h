#pragma once
#include <iostream>
#include <fstream>
#include <SDL3/SDL.h>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <tiny_obj_loader.h>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Audio.hpp>

#include "Smasher/Base.h"

#define SMASHER_RESOURCE_TYPE(type) ResourceType GetType() const { return GetStaticType();} \
static constexpr ResourceType GetStaticType() { return type; } \


namespace Smasher {
	struct SMASHER_API ResourceManifestInfo {
		const ResourceId resourceId;
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
			m_Loaded = true;
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
			if (!m_Font.openFromFile(m_Paths[0].string())) {
				throw Exceptions::ResourceFailedToLoad(std::format("Failed to load file {}", m_Paths[0].string()));
			}
			m_Loaded = true;
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
			m_Loaded = true;
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
			bool ret = false;
			switch (numPaths) {
			case 1:
				ret = m_Shader.loadFromFile(m_Paths[0].string(), sf::Shader::Type::Fragment);
				break;
			case 2:
				ret = m_Shader.loadFromFile(m_Paths[0].string(), m_Paths[1].string());
				break;
			case 3:
				ret = m_Shader.loadFromFile(
					m_Paths[0].string(),
					m_Paths[1].string(),
					m_Paths[2].string());
				break;
			default:
				throw Smasher::Exceptions::ResourceInvalidNumPaths("numPaths is invalid");
				assert(false); // Never should be reached
			}
			m_Loaded = ret;
		}

		ShaderResource(ResourceId id, const std::string &vert, const std::string &frag) :
			Resource(id), m_ShaderType(sf::Shader::Type::Geometry) {
			m_Loaded = m_Shader.loadFromMemory(vert, frag);
		}

		sf::Shader& GetShader() { return m_Shader; }

	private:
		sf::Shader m_Shader;
		sf::Shader::Type m_ShaderType = sf::Shader::Type::Fragment;
	};

	SDL_GPUShader *LoadGPUShaderHLSL(SDL_GPUDevice *device, SDL_GPUShaderStage stage, const char* code, const char* includeDir = NULL, const char* debugName = NULL);
	SDL_GPUComputePipeline* LoadComputePipelineHLSL(SDL_GPUDevice* device, const char* code, const char* includeDir = NULL, const char *debugName = NULL);

	class SMASHER_API SDLGraphicShaderResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::SHADER)
			SDLGraphicShaderResource() = delete;

		SDLGraphicShaderResource(ResourceId id,
			const ResourcePath* const relativePaths, const std::size_t numPaths,
			const ResourcePath& resourcesDirectory, std::shared_ptr<SDL_GPUDeviceWrapper> gpu, SDL_GPUShaderStage type);

		SDLGraphicShaderResource(ResourceId id, const char* code, std::optional<ResourcePath> includePath, std::shared_ptr<SDL_GPUDeviceWrapper> gpu, SDL_GPUShaderStage type, const char* debugName = NULL);


		SDL_GPUShader* GetShader();

		~SDLGraphicShaderResource();

	private:
		SDL_GPUShader* m_ShaderPtr;
		std::shared_ptr<SDL_GPUDeviceWrapper> m_GPUPtr;
		const SDL_GPUShaderStage m_ShaderType;
	};

	class SMASHER_API SDLComputeShaderResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::SHADER)
			SDLComputeShaderResource() = delete;

		SDLComputeShaderResource(ResourceId id,
			const ResourcePath* const relativePaths, const std::size_t numPaths,
			const ResourcePath& resourcesDirectory, std::shared_ptr<SDL_GPUDeviceWrapper> gpu);

		SDLComputeShaderResource(ResourceId id, const char *code, std::optional<ResourcePath> includePath, std::shared_ptr<SDL_GPUDeviceWrapper> gpu, const char *debugName = NULL);


		SDL_GPUComputePipeline* GetShader();

		~SDLComputeShaderResource();

	private:
		SDL_GPUComputePipeline* m_ShaderPtr;
		std::shared_ptr<SDL_GPUDeviceWrapper> m_GPUPtr;
	};

	class SMASHER_API FileResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::FILE)
		FileResource() = delete;
		FileResource(ResourceId id,
			const ResourcePath *const relativePaths, const std::size_t numPaths,
			const ResourcePath &resourcesDirectory, std::ios_base::openmode mode) :
			Resource(id, relativePaths, numPaths, resourcesDirectory), m_File(m_Paths[0], mode) {
			m_Loaded = true;
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

	struct VertexData {
		float
			px, py, pz, // Position
			nx, ny, nz, // Normal
			u, v;		// UV 
	};

	class SMASHER_API StaticMeshResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::STATIC_MESH)
			StaticMeshResource() = delete;
		StaticMeshResource(ResourceId id,
			const ResourcePath* const relativePaths, const std::size_t numPaths,
			const ResourcePath& resourcesDirectory, std::shared_ptr<SDL_GPUDeviceWrapper> gpu);
		~StaticMeshResource();

		SDL_GPUBuffer *GetVertexBuffer() const;
		SDL_GPUBuffer *GetIndexBuffer() const;
		unsigned int GetNumIndices() const;
		unsigned int GetNumVertices() const;
		glm::vec3 GetMinAABB() const;
		glm::vec3 GetMaxAABB() const;

	private:
		SDL_GPUBuffer *m_VertexBuffer;
		SDL_GPUBuffer *m_IndexBuffer;
		unsigned int m_NumIndices = 0;
		unsigned int m_NumVertices = 0;
		std::vector<VertexData> m_Vertices;
		std::vector<uint32_t> m_Indices;
		std::shared_ptr<SDL_GPUDeviceWrapper> m_GPUPtr;
		glm::vec3 m_MinAABB;
		glm::vec3 m_MaxAABB;
	};

	class SMASHER_API MaterialResource : public Resource {
	public:
		SMASHER_RESOURCE_TYPE(ResourceType::MATERIAL)
			MaterialResource() = delete;
		MaterialResource(ResourceId id,
			const ResourcePath* const relativePaths, const std::size_t numPaths,
			const ResourcePath& resourcesDirectory, std::shared_ptr<SDL_GPUDeviceWrapper> gpu);
		~MaterialResource();

		glm::uvec2 GetDimensions();
		Expected<void *> GetAlbedoTexture();
		Expected<void *> GetSpecularTexture();
		Expected<void *> GetNormalsTexture();

		glm::vec3 GetBaseAlbedo();
		glm::vec3 GetBaseSpecular();

	private:
		tinyobj::material_t m_Material;
		SDL_GPUTexture* m_UVTexture;
		SDL_GPUTexture* m_NormalTexture;
		SDL_GPUTexture* m_AlbedoTexture;
		SDL_Surface *m_AlbedoSurface = nullptr;
		std::shared_ptr<SDL_GPUDeviceWrapper> m_GPUPtr;
		glm::uvec2 m_Dimensions;
	};

	struct SMASHER_API MaterialBinding {
		const std::shared_ptr<MaterialResource> resource;
		const ResourceId materialId;
		const uint32_t index;
	};

	struct SMASHER_API StaticMeshBinding {
		const std::shared_ptr<StaticMeshResource> resource;
		const ResourceId meshId;
		const uint32_t index;
	};
}