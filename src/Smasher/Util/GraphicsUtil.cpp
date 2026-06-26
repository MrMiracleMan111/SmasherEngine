#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>
#include "Smasher/Base.h"
#include "Smasher/Util/GraphicsUtil.h"
#include "Smasher/ResourceManager.h"

namespace Smasher {
	namespace GraphicsUtil {
		GPURadixSortPool::GPURadixSortPool() :
			m_RadixBuffer1(NULL),
			m_RadixBuffer2(NULL),
			m_CounterBuffer(NULL),
			m_PrefixSumBuffer(NULL),
			m_InputBuffer(m_RadixBuffer1),
			m_OutputBuffer(m_RadixBuffer2)
		{};

		GPURadixSortPool::GPURadixSortPool(std::shared_ptr<SDL_GPUDeviceWrapper> gpu, ResourceManager& resourceMgr, int maxElements) :
			m_GPU(gpu),
			m_MaxElements(maxElements),
			m_InputBuffer(m_RadixBuffer1),
			m_OutputBuffer(m_RadixBuffer2)
		{
			m_CountCompShader = resourceMgr.GetOrLoadResource<Manifest::Shaders::GPUSort::radix_count_shader, Smasher::SDLComputeShaderResource>(gpu);
			m_PrefixSumCompShader = resourceMgr.GetOrLoadResource<Manifest::Shaders::GPUSort::radix_prefix_sum_shader, Smasher::SDLComputeShaderResource>(gpu);
			m_ReorderCompShader = resourceMgr.GetOrLoadResource<Manifest::Shaders::GPUSort::radix_reorder_shader, Smasher::SDLComputeShaderResource>(gpu);

			// Create Buffers
			{
				SDL_GPUBufferCreateInfo radixEntryBufferInfo{};
				radixEntryBufferInfo.size = m_MaxElements * sizeof(RadixEntry);
				radixEntryBufferInfo.usage = SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_READ | SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_WRITE;
				m_RadixBuffer1 = SDL_CreateGPUBuffer(m_GPU->Get(), &radixEntryBufferInfo);
				SDL_SetGPUBufferName(m_GPU->Get(), m_RadixBuffer1, "Radix Buffer 1");
				m_RadixBuffer2 = SDL_CreateGPUBuffer(m_GPU->Get(), &radixEntryBufferInfo);
				SDL_SetGPUBufferName(m_GPU->Get(), m_RadixBuffer2, "Radix Buffer 2");

				SDL_GPUBufferCreateInfo radixCounterBufferInfo{};
				radixCounterBufferInfo.size = NUM_SECTIONS * sizeof(SplitTable);
				radixCounterBufferInfo.usage = SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_READ | SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_WRITE;
				m_CounterBuffer = SDL_CreateGPUBuffer(m_GPU->Get(), &radixCounterBufferInfo);
				SDL_SetGPUBufferName(m_GPU->Get(), m_CounterBuffer, "Counter Buffer");

				SDL_GPUBufferCreateInfo radixPrefixSumBufferInfo{};
				radixPrefixSumBufferInfo.size = NUM_SECTIONS * sizeof(SplitTable);
				radixPrefixSumBufferInfo.usage = SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_READ | SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_WRITE;
				m_PrefixSumBuffer = SDL_CreateGPUBuffer(m_GPU->Get(), &radixPrefixSumBufferInfo);
				SDL_SetGPUBufferName(m_GPU->Get(), m_PrefixSumBuffer, "Prefix Sum Buffer");
			}
		};

		GPURadixSortPool::GPURadixSortPool(GPURadixSortPool&& other) noexcept :
			m_GPU(std::move(other.m_GPU)),
			m_MaxElements(other.m_MaxElements),
			m_CountCompShader(std::move(m_CountCompShader)),
			m_PrefixSumCompShader(std::move(m_PrefixSumCompShader)),
			m_ReorderCompShader(std::move(m_ReorderCompShader)),
			m_RadixBuffer1(other.m_RadixBuffer1),
			m_RadixBuffer2(other.m_RadixBuffer2),
			m_InputBuffer(m_RadixBuffer1),
			m_OutputBuffer(m_RadixBuffer2)
		{
			m_CounterBuffer = other.m_CounterBuffer;
			m_PrefixSumBuffer = other.m_PrefixSumBuffer;

			if (&other.m_InputBuffer.get() == &other.m_RadixBuffer2) {
				m_InputBuffer = m_RadixBuffer2;
				m_OutputBuffer = m_RadixBuffer1;
			}

			other.m_CounterBuffer = NULL;
			other.m_PrefixSumBuffer = NULL;
			other.m_RadixBuffer1 = NULL;
			other.m_RadixBuffer2 = NULL;
		};

		GPURadixSortPool& GPURadixSortPool::operator =(GPURadixSortPool&& other) noexcept {
			if (&other != this) {
				m_GPU = std::move(other.m_GPU);
				m_MaxElements = other.m_MaxElements;

				m_CountCompShader = std::move(other.m_CountCompShader); 
				m_PrefixSumCompShader = std::move(other.m_PrefixSumCompShader); 
				m_ReorderCompShader = std::move(other.m_ReorderCompShader);

				m_CounterBuffer = other.m_CounterBuffer;
				m_PrefixSumBuffer = other.m_PrefixSumBuffer;
				m_RadixBuffer1 = other.m_RadixBuffer1;
				m_RadixBuffer2 = other.m_RadixBuffer2;

				if (&other.m_InputBuffer.get() == &other.m_RadixBuffer2) {
					m_InputBuffer = m_RadixBuffer2;
					m_OutputBuffer = m_RadixBuffer1;
				}

				other.m_CounterBuffer = NULL;
				other.m_PrefixSumBuffer = NULL;
				other.m_RadixBuffer1 = NULL;
				other.m_RadixBuffer2 = NULL;
			}
			return *this;
		};

		GPURadixSortPool::~GPURadixSortPool() {
			if (!m_GPU) {
				return;
			}
			SDL_ReleaseGPUBuffer(m_GPU->Get(), m_CounterBuffer);
			SDL_ReleaseGPUBuffer(m_GPU->Get(), m_PrefixSumBuffer);
			SDL_ReleaseGPUBuffer(m_GPU->Get(), m_RadixBuffer1);
			SDL_ReleaseGPUBuffer(m_GPU->Get(), m_RadixBuffer2);
		};

		struct UBO {
			int numEntries;
			int splitTableIndex; // 0 1 2 3 4 5 6 7 8
			uint32_t padding1;
			uint32_t padding2;
		};

		void GPURadixSortPool::SortGPUBuffer(SDL_GPUCommandBuffer *commandBuffer, int numElements) {
			ZoneScoped;

			UBO ubo {
				.numEntries = numElements,
				.splitTableIndex = 0,
				.padding1 = 0,
				.padding2 = 0
			};


			std::vector<std::array<SDL_GPUStorageBufferReadWriteBinding, 3>> rwBufferBindings(NUM_SPLIT_TABLES_PER_SECTION);
			std::vector<std::array<SDL_GPUBuffer*, 1>> readBufferBindings(NUM_SPLIT_TABLES_PER_SECTION);

			SDL_GPUComputePass* computePass = NULL;
			for (int i = 0; i < NUM_SPLIT_TABLES_PER_SECTION; ++i) {
				ZoneScoped("Radix Pass");
				rwBufferBindings.push_back({
					SDL_GPUStorageBufferReadWriteBinding{ .buffer = m_CounterBuffer, .cycle = false},
					SDL_GPUStorageBufferReadWriteBinding{ .buffer = m_PrefixSumBuffer, .cycle = false},
					SDL_GPUStorageBufferReadWriteBinding{ .buffer = m_OutputBuffer.get(), .cycle = false}
				});
				readBufferBindings.push_back({ m_InputBuffer.get()});

				
				ubo.splitTableIndex = i;
				SDL_PushGPUComputeUniformData(commandBuffer, 0, &ubo, sizeof(ubo));
				
				computePass = SDL_BeginGPUComputePass(commandBuffer, NULL, 0, rwBufferBindings.back().data(), 3);
				SDL_BindGPUComputePipeline(computePass, m_CountCompShader->GetShader());
				SDL_BindGPUComputeStorageBuffers(computePass, 0, readBufferBindings.back().data(), 1);
				SDL_DispatchGPUCompute(computePass, NUM_SECTIONS, ROWS_PER_SPLIT_TABLE, 1);
				SDL_EndGPUComputePass(computePass);

				computePass = SDL_BeginGPUComputePass(commandBuffer, NULL, 0, rwBufferBindings.back().data(), 3);
				SDL_BindGPUComputePipeline(computePass, m_PrefixSumCompShader->GetShader());
				SDL_BindGPUComputeStorageBuffers(computePass, 0, readBufferBindings.back().data(), 1);
				SDL_DispatchGPUCompute(computePass, 1, 1, 1);
				SDL_EndGPUComputePass(computePass);

				computePass = SDL_BeginGPUComputePass(commandBuffer, NULL, 0, rwBufferBindings.back().data(), 3);
				SDL_BindGPUComputePipeline(computePass, m_ReorderCompShader->GetShader());
				SDL_BindGPUComputeStorageBuffers(computePass, 0, readBufferBindings.back().data(), 1);
				SDL_DispatchGPUCompute(computePass, NUM_SECTIONS, 1, 1);
				SDL_EndGPUComputePass(computePass);

				if (i % 2 == 0) {
					m_InputBuffer = m_RadixBuffer2;
					m_OutputBuffer = m_RadixBuffer1;
				}
				else {
					m_InputBuffer = m_RadixBuffer1;
					m_OutputBuffer = m_RadixBuffer2;
				}
			}
		};

		SDL_GPUBuffer* GPURadixSortPool::GetInputBuffer() const { return m_InputBuffer; };

		SDL_GPUBuffer* GPURadixSortPool::GetOutputBuffer() const { return m_OutputBuffer; };
	}
}
