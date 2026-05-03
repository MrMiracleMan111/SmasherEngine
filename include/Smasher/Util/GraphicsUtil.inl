#pragma once
namespace Smasher {
	template<class T>
	GPUBlockPool<T>::GPUBlockPool() {
		// Empty
	}

	template<class T>
	GPUBlockPool<T>::GPUBlockPool(std::shared_ptr<SDL_GPUDeviceWrapper> gpu, int blockSize, int blockCount) : 
		m_BlockSize(sizeof(T) * blockSize * blockCount)
	{
		SDL_GPUBufferCreateInfo instanceBufferInfo{};
		instanceBufferInfo.size = m_BlockSize;
		instanceBufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
		m_BlockBuffer = SDL_CreateGPUBuffer(gpu->Get(), &instanceBufferInfo);

		SDL_GPUTransferBufferCreateInfo transferBufferInfo{};
		transferBufferInfo.size = sizeof(T) * blockSize;
		transferBufferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
		m_TransferBuffer = SDL_CreateGPUTransferBuffer(gpu->Get(), &transferBufferInfo);

		for (int i = 0; i < blockCount; i++) {
			m_FreeBlockSlots.push(i);
		}
	}

	template<class T>
	GPUBlockPool<T>::GPUBlockPool(GPUBlockPool<T>& other) :
		m_BlockBuffer(other.m_BlockBuffer),
		m_BlockSize(other.m_BlockSize),
		m_FreeBlockSlots(other.m_FreeBlockSlots),
		m_GPU(other.m_GPU),
		m_TransferBuffer(other.m_TransferBuffer)
	{

	}

	template<class T>
	GPUBlockPool<T>::GPUBlockPool(GPUBlockPool<T>&& other) :
		m_BlockBuffer(std::move(other.m_BlockBuffer)),
		m_BlockSize(std::move(other.m_BlockSize)),
		m_FreeBlockSlots(std::move(other.m_FreeBlockSlots)),
		m_GPU(std::move(other.m_GPU)),
		m_TransferBuffer(std::move(other.m_TransferBuffer))
	{
		other.m_TransferBuffer = NULL;
		other.m_BlockBuffer = NULL;
	}

	template<class T>
	GPUBlockPool<T>& GPUBlockPool<T>::operator=(GPUBlockPool& other) {
		if (this != &other) {
			m_BlockBuffer = other.m_BlockBuffer;
			m_BlockSize = other.m_BlockSize;
			m_FreeBlockSlots = other.m_FreeBlockSlots;
			m_GPU = other.m_GPU;
			m_TransferBuffer = other.m_TransferBuffer;
			return *this;
		}
		return *this;
	}

	template<class T>
	GPUBlockPool<T>& GPUBlockPool<T>::operator=(GPUBlockPool&& other) {
		if (this != &other) {
			m_BlockBuffer = std::move(other.m_BlockBuffer);
			m_BlockSize = std::move(other.m_BlockSize);
			m_FreeBlockSlots = std::move(other.m_FreeBlockSlots);
			m_GPU = std::move(other.m_GPU);
			m_TransferBuffer = std::move(other.m_TransferBuffer);

			other.m_BlockBuffer = NULL;
			other.m_TransferBuffer = NULL;
		}
		return *this;
	}


	template<class T>
	GPUBlockPool<T>::~GPUBlockPool() {
		if (m_GPU) {
			if (m_BlockBuffer != NULL) {
				SDL_ReleaseGPUBuffer(m_GPU->Get(), m_BlockBuffer);
			}
			if (m_TransferBuffer != NULL) {
				SDL_ReleaseGPUTransferBuffer(m_GPU->Get(), m_TransferBuffer);
			}
		}
	}

	template<class T>
	Expected<GPUBlockHandle> GPUBlockPool<T>::AllocateBlock() {
		if (m_FreeBlockSlots.empty()) {
			return Expected<GPUBlockHandle>::Error(ERROR_GPUBlockPoolFull);
		}
		GPUBlockHandle handle;
		handle.index = m_FreeBlockSlots.front();
		m_FreeBlockSlots.pop();
		return handle;
	}

	template<class T>
	void GPUBlockPool<T>::FreeBlock(GPUBlockHandle block) {
		m_FreeBlockSlots.push(block.index);
	}

	// Overwrites data in instanceBuffer at the "index" slot
	// Writes instanceSize * batchSize bytes
	// Used to update instance data
	template<class T>
	SDL_GPUFence* GPUBlockPool<T>::WriteToBlockAsync(GPUBlockHandle block, const T* data) const {
		void* dst = SDL_MapGPUTransferBuffer(m_GPU->Get(), m_TransferBuffer, true);
		SDL_memcpy(dst, data, sizeof(T) * m_BlockSize);
		SDL_UnmapGPUTransferBuffer(m_GPU->Get(), m_TransferBuffer);

		SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(m_GPU->Get());
		SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

		SDL_GPUTransferBufferLocation location{};
		location.transfer_buffer = m_TransferBuffer;
		location.offset = 0;

		SDL_GPUBufferRegion region{};
		region.buffer = m_BlockBuffer;
		region.size = sizeof(T) * m_BlockSize;
		region.offset = block.index * sizeof(T) * m_BlockSize;

		SDL_UploadToGPUBuffer(copyPass, &location, &region, true);
		SDL_EndGPUCopyPass(copyPass);
		return SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
	}

	template<class T>
	void GPUBlockPool<T>::WriteToBlock(GPUBlockHandle block, const T* data) const {
		SDL_GPUFence* fence = WriteToBlockAsync(block, data);
		SDL_WaitForGPUFences(m_GPU->Get(), true, &fence, 1);
		SDL_ReleaseGPUFence(m_GPU->Get(), fence);
	}

	template<class T>
	SDL_GPUBuffer* GPUBlockPool<T>::GetBuffer() {
		return m_BlockBuffer;
	}
}
