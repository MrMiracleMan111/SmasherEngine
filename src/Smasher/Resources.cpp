#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <tiny_obj_loader.h>
#include "Smasher/Resources.h"

namespace Smasher {
	static const char* ENTRY_POINT_VERT = "VSMain";
	static const char* ENTRY_POINT_FRAG = "PSMain";
	static const char* ENTRY_POINT_COMP = "CSMain";

	static SDL_GPUShader* LoadGPUShaderHLSL(SDL_GPUDevice* device, SDL_GPUShaderStage stage, const char* code) {
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

		SDL_ShaderCross_HLSL_Info graphicShaderInfo
		{
			code,										/**< The HLSL source code for the shader. */
			entryPoint,									/**< The entry point function name for the shader in UTF-8. */
			NULL,									    /**< The include directory for shader code. Optional, can be NULL. */
			NULL,									    /**< An array of defines. Optional, can be NULL. If not NULL, must be terminated with a fully NULL define struct. */
			shaderCrossStage,							/**< The shader stage to compile the shader with. */
			0						                    /**< A properties ID for extensions. Should be 0 if no extensions are needed. */
		};

		size_t size;
		Uint8* shaderBytecode = (Uint8*)SDL_ShaderCross_CompileSPIRVFromHLSL(&graphicShaderInfo, &size);

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
			
		m_ShaderPtr = LoadGPUShaderHLSL(m_GPUPtr->Get(), type, (const char*)(file));

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
		SDL_GPUBuffer **pVertexPositionBuffer;
		SDL_GPUBuffer **pVertexNormalBuffer;
		SDL_GPUBuffer **pIndexBuffer;
		unsigned int *pNumIndices;
		unsigned int *pNumVertices;
	};

	static void LoadOBJMesh(SDL_GPUDevice *device, const LoadOBJMeshInfo &info) {
		tinyobj::ObjReaderConfig readerConfig;
		tinyobj::ObjReader reader;
		readerConfig.mtl_search_path = "./";
		readerConfig.triangulate = true;

		if (!reader.ParseFromFile(info.filepath, readerConfig)) {
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

		SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device);

		auto& attrib = reader.GetAttrib();
		auto& shapes = reader.GetShapes();
		auto& materials = reader.GetMaterials();

		size_t VERT_POS_BUFFER_SIZE = attrib.vertices.size() * sizeof(tinyobj::real_t);
		size_t VERT_NORM_BUFFER_SIZE = attrib.normals.size() * sizeof(tinyobj::real_t);
		size_t INDEX_BUFFER_SIZE = 0;
		unsigned int indexCount = 0;
		for (auto& shape : shapes) {
			indexCount += shape.mesh.indices.size();
			INDEX_BUFFER_SIZE += shape.mesh.indices.size() * sizeof(tinyobj::index_t);
		}
		*info.pNumIndices = indexCount;
		*info.pNumVertices = attrib.vertices.size();

		size_t MAX_BUFFER_SIZE = std::max({ VERT_POS_BUFFER_SIZE, VERT_NORM_BUFFER_SIZE, INDEX_BUFFER_SIZE });

		SDL_GPUTransferBuffer* transferPositionBuffer = NULL;
		SDL_GPUTransferBuffer* transferNormalBuffer = NULL;
		SDL_GPUTransferBuffer* transferIndexBuffer = NULL;

		// Create Vertex Position + Vertex Normals + Index Buffer
		{
			SDL_GPUBufferCreateInfo vertexPositionBufferInfo{};
			vertexPositionBufferInfo.size = VERT_POS_BUFFER_SIZE;
			vertexPositionBufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
			*info.pVertexPositionBuffer = SDL_CreateGPUBuffer(device, &vertexPositionBufferInfo);
			
			SDL_GPUBufferCreateInfo vertexNormalBufferInfo{};
			vertexNormalBufferInfo.size = VERT_NORM_BUFFER_SIZE;
			vertexNormalBufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
			*info.pVertexNormalBuffer = SDL_CreateGPUBuffer(device, &vertexNormalBufferInfo);
			
			SDL_GPUBufferCreateInfo indexBufferInfo{};
			indexBufferInfo.size = INDEX_BUFFER_SIZE;
			indexBufferInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;
			*info.pIndexBuffer = SDL_CreateGPUBuffer(device, &indexBufferInfo);
		}

		// Create Transfer Buffer, upload to vertex buffers and index buffer
		{
			SDL_GPUTransferBufferCreateInfo transferPositionBufferInfo {};
			transferPositionBufferInfo.size = VERT_POS_BUFFER_SIZE;
			transferPositionBufferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
			transferPositionBuffer = SDL_CreateGPUTransferBuffer(device, &transferPositionBufferInfo);
			SDL_GPUTransferBufferCreateInfo transferNormalBufferInfo{};
			transferNormalBufferInfo.size = VERT_NORM_BUFFER_SIZE;
			transferNormalBufferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
			transferNormalBuffer = SDL_CreateGPUTransferBuffer(device, &transferNormalBufferInfo);
			SDL_GPUTransferBufferCreateInfo transferIndexBufferInfo{};
			transferIndexBufferInfo.size = INDEX_BUFFER_SIZE;
			transferIndexBufferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
			transferIndexBuffer = SDL_CreateGPUTransferBuffer(device, &transferIndexBufferInfo);

			SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);


			// Upload Vertex Positions to Transfer Buffer
			{
				void* data = SDL_MapGPUTransferBuffer(device, transferPositionBuffer, false);
				SDL_memcpy(data, attrib.vertices.data(), VERT_POS_BUFFER_SIZE);

				SDL_GPUTransferBufferLocation location{};
				location.transfer_buffer = transferPositionBuffer;
				location.offset = 0;

				SDL_GPUBufferRegion region{};
				region.buffer = *info.pVertexPositionBuffer;
				region.size = VERT_POS_BUFFER_SIZE;
				region.offset = 0;

				SDL_UnmapGPUTransferBuffer(device, transferPositionBuffer);
				SDL_UploadToGPUBuffer(copyPass, &location, &region, true);
			}
			
			// Upload Vertex Normals to Transfer Buffer
			{
				void* data = SDL_MapGPUTransferBuffer(device, transferNormalBuffer, false);
				SDL_memcpy(data, attrib.normals.data(), VERT_POS_BUFFER_SIZE);

				SDL_GPUTransferBufferLocation location{};
				location.transfer_buffer = transferNormalBuffer;
				location.offset = 0;

				SDL_GPUBufferRegion region{};
				region.buffer = *info.pVertexNormalBuffer;
				region.size = VERT_POS_BUFFER_SIZE;
				region.offset = 0;

				SDL_UnmapGPUTransferBuffer(device, transferNormalBuffer);
				SDL_UploadToGPUBuffer(copyPass, &location, &region, true);
			}


			// Upload index data from all shapes to Index Buffer
			{
				void* data = SDL_MapGPUTransferBuffer(device, transferIndexBuffer, false);

				unsigned int indexCount = 0;
				for (auto& shape : shapes) {
					indexCount += shape.mesh.indices.size();
				}

				// Create tightly packed array of position indices
				std::vector<Uint32> positionIndices;
				positionIndices.resize(indexCount);
				std::vector<Uint32>::iterator itr = positionIndices.begin();
				for (auto& shape : shapes) {
					itr = std::transform(shape.mesh.indices.begin(), shape.mesh.indices.end(), positionIndices.begin(), [](const tinyobj::index_t& index) {
						return index.vertex_index;
					});
				};
				SDL_memcpy(data, positionIndices.data(), indexCount * sizeof(Uint32));


				SDL_GPUTransferBufferLocation location{};
				location.transfer_buffer = transferIndexBuffer;
				location.offset = 0;

				SDL_GPUBufferRegion region{};
				region.buffer = *info.pIndexBuffer;
				region.size = INDEX_BUFFER_SIZE;
				region.offset = 0;

				SDL_UnmapGPUTransferBuffer(device, transferIndexBuffer);
				SDL_UploadToGPUBuffer(copyPass, &location, &region, true);
			}


			SDL_EndGPUCopyPass(copyPass);
		}

		SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
		SDL_WaitForGPUFences(device, true, &fence, 1);
		SDL_ReleaseGPUTransferBuffer(device, transferPositionBuffer);
		SDL_ReleaseGPUTransferBuffer(device, transferNormalBuffer);
		SDL_ReleaseGPUTransferBuffer(device, transferIndexBuffer);
	}

	StaticMeshResource::StaticMeshResource(ResourceId id,
		const ResourcePath* const relativePaths, const std::size_t numPaths,
		const ResourcePath& resourcesDirectory, std::shared_ptr<SDL_GPUDeviceWrapper> gpu) :
		Resource(id, relativePaths, numPaths, resourcesDirectory),
		m_VertexPositionBuffer(NULL),
		m_VertexNormalBuffer(NULL),
		m_IndexBuffer(NULL),
		m_GPUPtr(gpu)
	{
		assert(m_Paths.size() == 1 && "Static mesh can only have one path");
		LoadOBJMeshInfo info = {
			.filepath = m_Paths.front().generic_string(),
			.pVertexPositionBuffer = &m_VertexPositionBuffer,
			.pVertexNormalBuffer = &m_VertexNormalBuffer,
			.pIndexBuffer = &m_IndexBuffer,
			.pNumIndices = &m_NumIndices,
			.pNumVertices = &m_NumVertices
		};
		LoadOBJMesh(*gpu.get(), info);
		m_Loaded = true;
	}

	StaticMeshResource::~StaticMeshResource() {
		SDL_ReleaseGPUBuffer(*m_GPUPtr, m_VertexPositionBuffer);
		SDL_ReleaseGPUBuffer(*m_GPUPtr, m_VertexNormalBuffer);
		SDL_ReleaseGPUBuffer(*m_GPUPtr, m_IndexBuffer);
	};

	SDL_GPUBuffer* StaticMeshResource::GetVertexPositionBuffer() const {
		return m_VertexPositionBuffer;
	};

	SDL_GPUBuffer* StaticMeshResource::GetVertexNormalBuffer() const {
		return m_VertexNormalBuffer;
	};

	SDL_GPUBuffer* StaticMeshResource::GetIndexBuffer() const {
		return m_IndexBuffer;
	};

	unsigned int StaticMeshResource::GetNumIndices() const {
		return m_NumIndices;
	}
	unsigned int StaticMeshResource::GetNumVertices() const {
		return m_NumVertices;
	}

}
