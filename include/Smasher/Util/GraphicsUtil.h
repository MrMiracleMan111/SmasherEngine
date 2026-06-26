#pragma once
#include "Smasher/Base.h"
#include <queue>
#include <SDL3/SDL.h>
#include "Smasher/ErrorCodes.h"
#include "Smasher/Exceptions.h"
#include "Smasher/ResourceManager.h"
#include "Smasher/Resources.h"
#include "Manifest.h"

namespace Smasher {
	namespace GraphicsUtil {
		// Pool of fixed size memory blocks
		struct GPUBlockHandle {
			int index = -1;
		};

		template<class T>
		class GPUBlockPool {
		public:
			GPUBlockPool();
			GPUBlockPool(std::shared_ptr<SDL_GPUDeviceWrapper> gpu, int blockSize, int blockCount, std::string debugName = "");
			GPUBlockPool(GPUBlockPool<T> &other);
			GPUBlockPool(GPUBlockPool<T> &&other);
			GPUBlockPool<T>& operator =(GPUBlockPool& other);
			GPUBlockPool<T>& operator =(GPUBlockPool &&other);
			~GPUBlockPool();
			Expected<GPUBlockHandle> AllocateBlock();	// Request a block
			void FreeBlock(GPUBlockHandle block);	// Mark block at index as unused
			SDL_GPUBuffer* GetBuffer();

			// Asynchronously overwrites data in instanceBuffer at the "index" slot
			// Writes instanceSize * batchSize bytes
			// Used to update instance data
			SDL_GPUFence* WriteToBlockAsync(GPUBlockHandle block, const T* data) const;
			void WriteToBlock(GPUBlockHandle block, const T* data) const;

		private:
			int m_BlockSize; // Number of entries per block
			std::queue<int> m_FreeBlockSlots;
			std::string m_DebugName;

			std::shared_ptr<SDL_GPUDeviceWrapper> m_GPU;
			SDL_GPUBuffer* m_BlockBuffer; // Mega buffer for all static meshes
			SDL_GPUTransferBuffer* m_TransferBuffer; // For transfering data to instance buffer
		};

		static const int NUM_SECTIONS = 32;
		static const int BITS_PER_SPLIT = 4;
		static const int NUM_REORDER_PASSES = 32 / BITS_PER_SPLIT; // 8
		static const int NUM_SPLIT_TABLES_PER_SECTION = 32 / BITS_PER_SPLIT; // 8
		static const int ROWS_PER_SPLIT_TABLE = 1U << BITS_PER_SPLIT; // 16 possible values for 4 bit splits
		static const int SIZE_SECTION = ROWS_PER_SPLIT_TABLE; // 8 * 16 int entries per section
		static const int THREADS_PER_SPLIT_TABLE_ROW = 32;
		static const int MAX_RADIX_SORT_ENTRIES = 1024 * 16;

		// To be sorted by "key" value
		// Contains "Data"
		struct RadixEntry {
			uint32_t key;
			int data;
		};

		struct SplitTable
		{
			int row[ROWS_PER_SPLIT_TABLE];
		};


		// GPU Radix Sort Implementation from this paper: https://gpuopen.com/download/Introduction_to_GPU_Radix_Sort.pdf
		// To speedup Prefix Sum this website provides guidance: https://developer.nvidia.com/gpugems/gpugems3/part-vi-gpu-computing/chapter-39-parallel-prefix-sum-scan-cuda
		class GPURadixSortPool {
		public:
			static const int INPUT_BUFFER_SPLITS = 8; // How many threads to split initial buffer into (needs to match compute shader)
			static const int BITS_PER_SPLIT = 8; // How many bits per radix split (ex. 8 bits = 256 buckets)
			GPURadixSortPool();
			GPURadixSortPool(std::shared_ptr<SDL_GPUDeviceWrapper> gpu, ResourceManager& resourceMgr, int maxElements);
			GPURadixSortPool(GPURadixSortPool& other) = delete;
			GPURadixSortPool(GPURadixSortPool&& other) noexcept;
			GPURadixSortPool& operator =(GPURadixSortPool& other) = delete;
			GPURadixSortPool& operator =(GPURadixSortPool&& other) noexcept;
			~GPURadixSortPool();

			void SortGPUBuffer(SDL_GPUCommandBuffer* commandBuffer, int numElements);
			SDL_GPUBuffer* GetInputBuffer() const;
			SDL_GPUBuffer* GetOutputBuffer() const;
		private:
			// The three passes needed for every "BITS_PER_SPLIT" bits (ex. every 8 bits) 
			std::shared_ptr<SDLComputeShaderResource> m_CountCompShader;
			std::shared_ptr<SDLComputeShaderResource> m_PrefixSumCompShader;
			std::shared_ptr<SDLComputeShaderResource> m_ReorderCompShader;
			std::reference_wrapper<SDL_GPUBuffer*> m_OutputBuffer;
			std::reference_wrapper<SDL_GPUBuffer*> m_InputBuffer;
			std::shared_ptr<SDL_GPUDeviceWrapper> m_GPU;
			SDL_GPUBuffer *m_RadixBuffer1 = NULL; // Buffer of unsorted inputs
			SDL_GPUBuffer *m_RadixBuffer2 = NULL; // Buffer of sorted inputs
			SDL_GPUBuffer *m_CounterBuffer = NULL; // Count of each Digit
			SDL_GPUBuffer *m_PrefixSumBuffer = NULL; // (Offsets within each section)

			int m_MaxElements;
		};
	}
}

#include "Smasher/Util/GraphicsUtil.inl"
