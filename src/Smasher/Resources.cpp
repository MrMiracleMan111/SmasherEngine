#include <functional>
#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <tiny_obj_loader.h>
#include "Smasher/Resources.h"
#include "Smasher/ErrorCodes.h"

namespace Smasher {
	static const char* ENTRY_POINT_VERT = "VSMain";
	static const char* ENTRY_POINT_FRAG = "PSMain";
	static const char* ENTRY_POINT_COMP = "CSMain";

	static SDL_GPUShader* LoadGPUShaderHLSL(SDL_GPUDevice* device, SDL_GPUShaderStage stage, const char* code, const char *debugName) {
		SDL_ShaderCross_ShaderStage shaderCrossStage;
		const char* entryPoint;
		switch (stage) {
			case SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_VERTEX:
				shaderCrossStage = SDL_ShaderCross_ShaderStage::SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
				entryPoint = ENTRY_POINT_VERT;
				break;
			case SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_FRAGMENT:
				shaderCrossStage = SDL_ShaderCross_ShaderStage::SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
				entryPoint = ENTRY_POINT_FRAG;
				break;
			default:
				assert(false && "Invalid SDL_GPUShaderStage");
				return NULL;
		}


		SDL_PropertiesID props = 0;
		if (debugName != NULL) {
			props = SDL_CreateProperties();
			SDL_SetBooleanProperty(props, SDL_SHADERCROSS_PROP_SHADER_DEBUG_ENABLE_BOOLEAN, true);
			//SDL_SetStringProperty(props, SDL_SHADERCROSS_PROP_SHADER_DEBUG_NAME_STRING, "TEST");
		}


		SDL_ShaderCross_HLSL_Info graphicShaderInfo
		{
			code,										/**< The HLSL source code for the shader. */
			entryPoint,									/**< The entry point function name for the shader in UTF-8. */
			NULL,									    /**< The include directory for shader code. Optional, can be NULL. */
			NULL,									    /**< An array of defines. Optional, can be NULL. If not NULL, must be terminated with a fully NULL define struct. */
			shaderCrossStage,							/**< The shader stage to compile the shader with. */
			props						                /**< A properties ID for extensions. Should be 0 if no extensions are needed. */
		};

		size_t size;
		Uint8* shaderBytecode = nullptr;
		
		shaderBytecode = (Uint8*)SDL_ShaderCross_CompileSPIRVFromHLSL(&graphicShaderInfo, &size);

		if (shaderBytecode == NULL) {
			std::cerr << "Error compiling SPIRV from HLSL: " << SDL_GetError() << std::endl;
			assert(false);
		}

		SDL_ShaderCross_SPIRV_Info shaderSPIRVInfo
		{
			shaderBytecode,							/**< The SPIRV bytecode. */
			size,									/**< The length of the SPIRV bytecode. */
			entryPoint,								/**< The entry point function name for the shader in UTF-8. */
			shaderCrossStage,						/**< The shader stage to transpile the shader with. */
			0										/**< A properties ID for extensions. Should be 0 if no extensions are needed. */
		};

		SDL_ShaderCross_GraphicsShaderMetadata* shaderMetadata = SDL_ShaderCross_ReflectGraphicsSPIRV(shaderBytecode, size, 0);

		assert(shaderMetadata != NULL);


		SDL_GPUShader *pShader = SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(device, &shaderSPIRVInfo, &shaderMetadata->resource_info, 0);

		assert(pShader != NULL);

		SDL_free(shaderMetadata);
		return pShader;
	}

	static SDL_GPUComputePipeline* LoadComputePipelineHLSL(SDL_GPUDevice* device, const char* code) {
		SDL_ShaderCross_HLSL_Info graphicShaderInfo
		{
			code,										/**< The HLSL source code for the shader. */
			ENTRY_POINT_COMP,									/**< The entry point function name for the shader in UTF-8. */
			NULL,									    /**< The include directory for shader code. Optional, can be NULL. */
			NULL,									    /**< An array of defines. Optional, can be NULL. If not NULL, must be terminated with a fully NULL define struct. */
			SDL_SHADERCROSS_SHADERSTAGE_COMPUTE,		/**< The shader stage to compile the shader with. */
			0						                    /**< A properties ID for extensions. Should be 0 if no extensions are needed. */
		};
		size_t size;
		Uint8* shaderBytecode = (Uint8*)SDL_ShaderCross_CompileSPIRVFromHLSL(&graphicShaderInfo, &size);

		if (shaderBytecode == NULL) {
			std::cerr << "Error Compiling HLSL: " << SDL_GetError() << std::endl;
			return NULL;
		}

		SDL_ShaderCross_SPIRV_Info shaderSPIRVInfo
		{
			shaderBytecode,                     /**< The SPIRV bytecode. */
			size,									/**< The length of the SPIRV bytecode. */
			"CSMain",									/**< The entry point function name for the shader in UTF-8. */
			SDL_SHADERCROSS_SHADERSTAGE_COMPUTE,		/**< The shader stage to transpile the shader with. */
			0										/**< A properties ID for extensions. Should be 0 if no extensions are needed. */
		};

		
		SDL_ShaderCross_ComputePipelineMetadata* shaderMetadata = SDL_ShaderCross_ReflectComputeSPIRV(shaderBytecode, size, 0);

		SDL_GPUComputePipeline* pShader = SDL_ShaderCross_CompileComputePipelineFromSPIRV(device, &shaderSPIRVInfo, shaderMetadata, 0);

		SDL_free(shaderMetadata);
		return pShader;
	}

	SDLGraphicShaderResource::SDLGraphicShaderResource(ResourceId id,
		const ResourcePath* const relativePaths, const std::size_t numPaths,
		const ResourcePath& resourcesDirectory, std::shared_ptr<SDL_GPUDeviceWrapper> gpu, SDL_GPUShaderStage type) :
		Resource(id, relativePaths, numPaths, resourcesDirectory),
		m_GPUPtr{ gpu },
		m_ShaderType{ type }
	{
		if (numPaths != 1) {
			throw Smasher::Exceptions::ResourceInvalidNumPaths("numPaths is invalid");
		}

		size_t fileSize = 0;
		char* file = (char*)SDL_LoadFile(m_Paths[0].string().c_str(), &fileSize);
		assert(file != NULL && "Error loading shader file, path may be invalid");
			
		std::string debugName = std::move(m_Paths[0].filename().generic_string());
		m_ShaderPtr = LoadGPUShaderHLSL(m_GPUPtr->Get(), type, (const char*)(file), "test");

		SDL_free(file);

		m_Loaded = (m_ShaderPtr != NULL);
		if (!m_Loaded) {
			std::cerr << "Error Loading Graphics Shader SDL Error: " << SDL_GetError() << "\n";
			throw Exceptions::ResourceFailedToLoad(std::format("Failed to load Shader {}", m_Paths[0].string()));
		}
	}

	SDL_GPUShader* SDLGraphicShaderResource::GetShader() { return m_ShaderPtr; }

	SDLGraphicShaderResource::~SDLGraphicShaderResource() {
		SDL_ReleaseGPUShader(m_GPUPtr->Get(), m_ShaderPtr);
	}

	SDLComputeShaderResource::SDLComputeShaderResource(ResourceId id,
		const ResourcePath* const relativePaths, const std::size_t numPaths,
		const ResourcePath& resourcesDirectory, std::shared_ptr<SDL_GPUDeviceWrapper> gpu) :
		Resource(id, relativePaths, numPaths, resourcesDirectory),
		m_GPUPtr{ gpu }
	{
		if (numPaths != 1) {
			throw Smasher::Exceptions::ResourceInvalidNumPaths("numPaths is invalid");
		}

		size_t fileSize = 0;
		char* file = (char*)SDL_LoadFile(m_Paths[0].string().c_str(), &fileSize);
		m_ShaderPtr = LoadComputePipelineHLSL(m_GPUPtr->Get(), file);
		m_Loaded = (m_ShaderPtr != NULL);

		SDL_free(file);
		if (!m_Loaded) {
			std::cerr << "Error Loading Compute Shader SDL Error: " << SDL_GetError() << "\n";
			throw Exceptions::ResourceFailedToLoad(std::format("Failed to load Shader {}", m_Paths[0].string()));
		}
	}

	SDL_GPUComputePipeline* SDLComputeShaderResource::GetShader() { return m_ShaderPtr; }

	SDLComputeShaderResource::~SDLComputeShaderResource() {
		SDL_ReleaseGPUComputePipeline(m_GPUPtr->Get(), m_ShaderPtr);
	}

	struct LoadOBJMeshInfo {
		std::string filepath;
		unsigned int *pNumIndices;
		unsigned int *pNumVertices;
		SDL_GPUBuffer** pVertexBuffer;
		std::vector<VertexData>* pVertexData;
		std::vector<uint32_t>* pIndices;
	};

	static struct IndexTupleHasher {
		size_t operator()(const std::tuple<int, int, int>& indices) const {
			int posIndex = std::get<0>(indices);
			int normIndex = std::get<1>(indices);
			int uvIndex = std::get<2>(indices);
			return std::hash<int>()(posIndex) ^ std::hash<int>()(normIndex) ^ std::hash<int>()(uvIndex);
		}
	};

	static void LoadOBJMesh(
	std::string filepath,
	std::vector<VertexData>* pVertices,
	std::vector<uint32_t>* pIndices
	) {
		tinyobj::ObjReaderConfig readerConfig;
		tinyobj::ObjReader reader;
		readerConfig.mtl_search_path = "./";
		readerConfig.triangulate = true;

		if (!reader.ParseFromFile(filepath, readerConfig)) {
			// Error loading mesh
			std::cerr << "Something went wrong while parsing obj file\n";

			if (!reader.Error().empty()) {
				std::cerr << "TinyObjReader Error: " << reader.Error() << std::endl;
			}
			return;
		}

		if (!reader.Warning().empty()) {
			std::cerr << "TinyObjReader Warning: " << reader.Warning() << std::endl;
		}
		auto& attrib = reader.GetAttrib();
		auto& shapes = reader.GetShapes();
		auto& materials = reader.GetMaterials();

		std::unordered_map<std::tuple<int, int, int>, uint32_t, IndexTupleHasher> indexMap;
		unsigned int indexCount = 0;
		for (auto& shape : shapes) {
			for (auto& idx : shape.mesh.indices) {
				auto key = std::make_tuple(idx.normal_index, idx.texcoord_index, idx.vertex_index);
				if (!indexMap.contains(key)) {
					indexMap[key] = pVertices->size();
					VertexData data = { 0, 0, 0, 0, 0, 0, 0, 0 };
					if (idx.vertex_index != -1) {
						data.px = attrib.vertices[3 * idx.vertex_index + 0]; // px
						data.py = attrib.vertices[3 * idx.vertex_index + 1]; // py
						data.pz = attrib.vertices[3 * idx.vertex_index + 2]; // pz
					}
					if (idx.normal_index != -1) {
						data.nx = attrib.normals[3 * idx.normal_index + 0]; // px
						data.ny = attrib.normals[3 * idx.normal_index + 1]; // py
						data.nz = attrib.normals[3 * idx.normal_index + 2]; // pz
					}
					if (idx.texcoord_index != -1) {
						data.u = attrib.texcoords[2 * idx.texcoord_index + 0];		  // u
						data.v = 1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]; // v
					}
					pVertices->emplace_back(data);
				}
				pIndices->push_back(indexMap[key]);
			}
		}
	}

	static void UploadOBJMeshToGPU(SDL_GPUDevice* device,
		const std::vector<VertexData>& vertices,
		const std::vector<uint32_t>& indices,
		SDL_GPUBuffer** pVertexBuffer,
		SDL_GPUBuffer** pIndexBuffer) {

		SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device);

		const size_t VERTEX_BUFFER_SIZE = vertices.size() * sizeof(VertexData);
		const size_t INDEX_BUFFER_SIZE = indices.size() * sizeof(uint32_t);

		SDL_GPUTransferBuffer* transferVertexBuffer = NULL;
		SDL_GPUTransferBuffer* transferIndexBuffer = NULL;

		// Create Vertex and Index Buffers
		{
			SDL_GPUBufferCreateInfo vertexBufferInfo{};
			vertexBufferInfo.size = vertices.size() * sizeof(VertexData);
			vertexBufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
			*pVertexBuffer = SDL_CreateGPUBuffer(device, &vertexBufferInfo);

			SDL_GPUBufferCreateInfo indexBufferInfo{};
			indexBufferInfo.size = indices.size() * sizeof(uint32_t);
			indexBufferInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;
			*pIndexBuffer = SDL_CreateGPUBuffer(device, &indexBufferInfo);
		}

		// Create Transfer Buffer, upload to vertex buffers and index buffer
		{
			SDL_GPUTransferBufferCreateInfo transferVertexBufferInfo{};
			transferVertexBufferInfo.size = VERTEX_BUFFER_SIZE;
			transferVertexBufferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
			transferVertexBuffer = SDL_CreateGPUTransferBuffer(device, &transferVertexBufferInfo);
			SDL_GPUTransferBufferCreateInfo transferIndexBufferInfo{};
			transferIndexBufferInfo.size = INDEX_BUFFER_SIZE;
			transferIndexBufferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
			transferIndexBuffer = SDL_CreateGPUTransferBuffer(device, &transferIndexBufferInfo);

			SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);


			// Upload Vertex (position, normal, uv) to Transfer Buffer
			{
				void* data = SDL_MapGPUTransferBuffer(device, transferVertexBuffer, false);
				SDL_memcpy(data, vertices.data(), VERTEX_BUFFER_SIZE);

				SDL_GPUTransferBufferLocation location{};
				location.transfer_buffer = transferVertexBuffer;
				location.offset = 0;

				SDL_GPUBufferRegion region{};
				region.buffer = *pVertexBuffer;
				region.size = VERTEX_BUFFER_SIZE;
				region.offset = 0;

				SDL_UnmapGPUTransferBuffer(device, transferVertexBuffer);
				SDL_UploadToGPUBuffer(copyPass, &location, &region, true);
			}

			// Upload index data from all shapes to Index Buffer
			{
				void* data = SDL_MapGPUTransferBuffer(device, transferIndexBuffer, false);
				SDL_memcpy(data, indices.data(), INDEX_BUFFER_SIZE);


				SDL_GPUTransferBufferLocation location{};
				location.transfer_buffer = transferIndexBuffer;
				location.offset = 0;

				SDL_GPUBufferRegion region{};
				region.buffer = *pIndexBuffer;
				region.size = INDEX_BUFFER_SIZE;
				region.offset = 0;

				SDL_UnmapGPUTransferBuffer(device, transferIndexBuffer);
				SDL_UploadToGPUBuffer(copyPass, &location, &region, true);
			}


			SDL_EndGPUCopyPass(copyPass);
		}

		SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
		SDL_WaitForGPUFences(device, true, &fence, 1);
		SDL_ReleaseGPUFence(device, fence);
		SDL_ReleaseGPUTransferBuffer(device, transferVertexBuffer);
		SDL_ReleaseGPUTransferBuffer(device, transferIndexBuffer);
	}
	
	StaticMeshResource::StaticMeshResource(ResourceId id,
		const ResourcePath* const relativePaths, const std::size_t numPaths,
		const ResourcePath& resourcesDirectory, std::shared_ptr<SDL_GPUDeviceWrapper> gpu) :
		Resource(id, relativePaths, numPaths, resourcesDirectory),
		m_VertexBuffer(NULL),
		m_IndexBuffer(NULL),
		m_GPUPtr(gpu)
	{
		assert(m_Paths.size() == 1 && "Static mesh can only have one path");
		std::vector<VertexData> vertices;
		std::vector<uint32_t> indices;
		LoadOBJMesh(m_Paths.front().generic_string(), &vertices, &indices);
		m_NumIndices = indices.size();
		m_NumVertices = vertices.size();
		UploadOBJMeshToGPU(gpu->Get(), vertices, indices, &m_VertexBuffer, &m_IndexBuffer);
		m_Loaded = true;
	}

	StaticMeshResource::~StaticMeshResource() {
		SDL_ReleaseGPUBuffer(*m_GPUPtr, m_VertexBuffer);
		SDL_ReleaseGPUBuffer(*m_GPUPtr, m_IndexBuffer);
	};

	SDL_GPUBuffer* StaticMeshResource::GetVertexBuffer() const {
		return m_VertexBuffer;
	}

	SDL_GPUBuffer* StaticMeshResource::GetIndexBuffer() const {
		return m_IndexBuffer;
	};

	unsigned int StaticMeshResource::GetNumIndices() const {
		return m_NumIndices;
	};

	unsigned int StaticMeshResource::GetNumVertices() const {
		return m_NumVertices;
	};

	MaterialResource::MaterialResource(ResourceId id,
		const ResourcePath* const relativePaths, const std::size_t numPaths,
		const ResourcePath& resourcesDirectory, std::shared_ptr<SDL_GPUDeviceWrapper> gpu) :
		Resource(id, relativePaths, numPaths, resourcesDirectory)
	{
		assert(m_Paths.size() == 1 && "Static mesh can only have one path");
		std::map<std::string, int> _{};
		std::vector<tinyobj::material_t> __{};
		std::ifstream stream{ m_Paths.front() };
		std::string warning, error;
		tinyobj::LoadMtl(&_, &__, &stream, &warning, &error);
		std::cerr << warning << std::endl;
		std::cerr << error << std::endl;
		m_Material = std::move(__.front());

		if (!m_Material.diffuse_texname.empty()) {
			std::filesystem::path tmp = m_Paths.front().parent_path();
			tmp.append(m_Material.diffuse_texname);
			m_AlbedoSurface = SDL_LoadPNG(tmp.generic_string().c_str());
			if (m_AlbedoSurface == nullptr) {
				std::cerr << "Error Loading PNG: " << tmp.generic_string().c_str() << "\n" << SDL_GetError() << std::endl;
			}
			m_Dimensions.x = static_cast<uint32_t>(m_AlbedoSurface->w);
			m_Dimensions.y = static_cast<uint32_t>(m_AlbedoSurface->h);
		}
	};

	MaterialResource::~MaterialResource() {
		SDL_DestroySurface(m_AlbedoSurface);
	};

	glm::uvec2 MaterialResource::GetDimensions() {
		return m_Dimensions;
	};

	Expected<void*> MaterialResource::GetAlbedo() {
		if (m_AlbedoSurface == nullptr) {
			return Expected<void*>::Error(ERROR_MaterialHasNoAlbedo);
		}
		return m_AlbedoSurface->pixels;
	};

	Expected<void*> MaterialResource::GetSpecular() {
		return nullptr;
	};

	Expected<void*> MaterialResource::GetNormals() {
		return nullptr;
	};

}
