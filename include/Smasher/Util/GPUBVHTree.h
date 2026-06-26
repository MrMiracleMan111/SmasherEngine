#include <SDL3/SDL.h>
#include "Smasher/Base.h"
#include "Smasher/Resources.h"
#include "Smasher/Util/GraphicsUtil.h"
#include "Smasher/ComponentSystems/StaticMeshSystem.h"

namespace Smasher {
	namespace GraphicsUtil {
		struct SMASHER_API BVHInstanceBufferInfo {
			SDL_GPUBuffer* buffer = NULL;
			glm::vec3 globalMinAABB;
			glm::vec3 globalMaxAABB;
			int offset; // Index
			int count; // Number of instances
		};

		struct SMASHER_API BVHDebugDrawOpts {
			bool highlightLeaves = true;
			int minLevel = 0; // Level of the tree to draw (0 = ROOT)
			int maxLevel = 0; // Level of the tree to draw
		};

		class SMASHER_API GPUBVHTree {
		public:
			inline static const float DEBUG_CUBE_VERTICES[]
			{
				-1.f, -1.f, -1.f, // 0: Bottom-Back-Left
				 1.f, -1.f, -1.f, // 1: Bottom-Back-Right
				 1.f,  1.f, -1.f, // 2: Top-Back-Right
				-1.f,  1.f, -1.f, // 3: Top-Back-Left
				-1.f, -1.f,  1.f, // 4: Bottom-Front-Left
				 1.f, -1.f,  1.f, // 5: Bottom-Front-Right
				 1.f,  1.f,  1.f, // 6: Top-Front-Right
				-1.f,  1.f,  1.f  // 7: Top-Front-Left
			};

			inline static const uint32_t DEBUG_CUBE_INDICES[]
			{
				// Back face
				0, 1, 1, 2, 2, 3, 3, 0,
				// Front face
				4, 5, 5, 6, 6, 7, 7, 4,
				// Edges
				0, 4, 1, 5, 2, 6, 3, 7
			};

			GPUBVHTree() = default;
			GPUBVHTree(std::shared_ptr<SDL_GPUDeviceWrapper> gpu);
			GPUBVHTree(GPUBVHTree&& other) = default;
			GPUBVHTree& operator= (GPUBVHTree&& other) = default;

			virtual void Construct(SDL_GPUCommandBuffer *commandBuffer, glm::vec3 globalMinAABB, glm::vec3 globalMaxAABB, int numEntries) = 0;
			virtual SDL_GPUBuffer* GetInternalNodes() = 0;
			virtual SDL_GPUBuffer* GetLeafNodes() = 0;
			virtual Expected<SDL_GPUFence*> DebugDraw(SDL_GPUTexture* depthTexture, SDL_GPUTexture* targetTexture, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, glm::uvec2 viewSize, int numEntries, BVHDebugDrawOpts& opts) = 0;
		protected:
			std::shared_ptr<SDL_GPUDeviceWrapper> m_GPU;
		};

		struct SMASHER_API BVHMortonInternalNode
		{
			glm::vec3 minAABB;
			int childCount;
			glm::vec3 maxAABB;
			uint32_t flags;
		};

		struct SMASHER_API BVHMortonLeafNode
		{
			glm::vec3 minAABB;
			int parentNode;
			glm::vec3 maxAABB;
			int instanceIndex;
		};

		class SMASHER_API GPUBVHTreeMorton : public GPUBVHTree {
		public:
			GPUBVHTreeMorton() = default;
			GPUBVHTreeMorton(std::shared_ptr<SDL_GPUDeviceWrapper> gpu, ResourceManager &resourceMgr, unsigned int maxEntries);
			GPUBVHTreeMorton(GPUBVHTreeMorton&& other) noexcept;
			GPUBVHTreeMorton& operator= (GPUBVHTreeMorton&& other) noexcept;
			~GPUBVHTreeMorton();

			// Constructs 
			void Initialize(SDL_GPUCommandBuffer* commandBuffer, const StaticMeshSystem::Context& staticMeshSysCtx, SDL_GPUBuffer* meshInstancesBuffer, SDL_GPUBuffer* meshPropsBuffer);
			void Construct(SDL_GPUCommandBuffer *commandBuffer, glm::vec3 globalMinAABB, glm::vec3 globalMaxAABB, int numEntries) override;
			Expected<SDL_GPUFence*> DebugDraw(SDL_GPUTexture* depthTexture, SDL_GPUTexture* targetTexture, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, glm::uvec2 viewSize, int numEntries, BVHDebugDrawOpts& opts) override;
			SDL_GPUBuffer* GetInternalNodes();
			SDL_GPUBuffer* GetLeafNodes();
			int GetMaxEntries();

		private:
			GraphicsUtil::GPURadixSortPool m_RadixSortPool;
			std::shared_ptr<SDLComputeShaderResource> m_RadixInputShader;
			std::shared_ptr<SDLComputeShaderResource> m_InitializeBVHShader;
			std::shared_ptr<SDLComputeShaderResource> m_ConstructBVHShader;
			std::shared_ptr<SDLGraphicShaderResource> m_DebugBVHFragShader;
			std::shared_ptr<SDLGraphicShaderResource> m_DebugBVHVertShader;
			SDL_GPUBuffer* m_BVHInternalNodes = NULL;
			SDL_GPUBuffer* m_BVHLeafNodes = NULL;
			SDL_GPUBuffer* m_DebugCubeVertexBuffer = NULL;
			SDL_GPUBuffer* m_DebugCubeIndexBuffer = NULL;
			SDL_GPUGraphicsPipeline* m_DebugBVHPipeline = NULL;
			int m_MaxEntries = 0;
		};

		class GPUBVHTreeSAH {

		};
	}
}
