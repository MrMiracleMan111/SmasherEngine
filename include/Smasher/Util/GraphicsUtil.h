#pragma once
#include "Smasher/Base.h"
#include <queue>
#include <SDL3/SDL.h>
#include "Smasher/ErrorCodes.h"
#include "Smasher/Exceptions.h"

namespace Smasher {
	// Pool of fixed size memory blocks
	struct GPUBlockHandle {
		int index = -1;
	};

	template<class T>
	class GPUBlockPool {
	public:
		GPUBlockPool();
		GPUBlockPool(std::shared_ptr<SDL_GPUDeviceWrapper> gpu, int blockSize, int blockCount);
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

		std::shared_ptr<SDL_GPUDeviceWrapper> m_GPU;
		SDL_GPUBuffer* m_BlockBuffer; // Mega buffer for all static meshes
		SDL_GPUTransferBuffer* m_TransferBuffer; // For transfering data to instance buffer
	};
}

#include "Smasher/Util/GraphicsUtil.inl"
