#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>
#include <iostream>
#include <thread>
#include "Smasher/Base.h"

namespace Smasher {

#ifdef BENCHMARK
	namespace InternalTimers {
		std::chrono::microseconds SMASHER_TimeAccumulator{}; // Used for time checking in benchmarks
		std::chrono::microseconds SMASHER_TimeSampleSum{}; // Used for time checking in benchmarks
		size_t SMASHER_TimeSampleCount{}; // Used for time checking in benchmarks
	}
#endif

	Degrees ToDegrees(Radians rad) { return rad * (180.f / (float)std::numbers::pi); };
	Radians ToRadians(Degrees deg) { return deg * ((float)std::numbers::pi / 180.f); };

	// Column major order Copy
	template<>
	void Matrix<3, 3>::CopyMatrix(const sf::Transform &transform) {
		const float *tmp = transform.getMatrix();
		for (int i = 0; i < 3; i++) {              // Row
			for (int j = 0; j < 2; j++) {          // Column
				array[i * 3 + j] = tmp[i * 4 + j];
			}
		}

		// Copy translation
		array[2 * 3 + 0] = tmp[3 * 4 + 0];
		array[2 * 3 + 1] = tmp[3 * 4 + 1];

		// Last Row is [0 0 1] to satify homogenous coordinates
		array[0 * 3 + 2] = 0.f;
		array[1 * 3 + 2] = 0.f;
		array[2 * 3 + 2] = 1.f;

	}

	template<>
	void Matrix<4, 4>::CopyMatrix(const sf::Transform &transform) {
		CopyMatrix(transform.getMatrix(), 4 * 4, array);
	}

	void PrecisionSleep(std::chrono::duration<double> duration) {
		static const auto BUFFER = std::chrono::milliseconds{ 3 };
		auto start = std::chrono::high_resolution_clock::now();
		auto end = start + duration;

		auto remaining = end - std::chrono::high_resolution_clock::now();

		if (remaining > BUFFER) {
			std::this_thread::sleep_for(remaining - BUFFER);
		}

		while (std::chrono::high_resolution_clock::now() < end) {
			std::this_thread::yield();
		}
	}

	SDL_GPUDeviceWrapper::~SDL_GPUDeviceWrapper() {
		if (m_GPUPtr != nullptr) {
			SDL_free(m_GPUPtr);
		}
	}

	SDL_GPUDeviceWrapper::SDL_GPUDeviceWrapper(SDL_GPUDevice* m_GPUPtr) :
		m_GPUPtr(m_GPUPtr)
	{
	}

	SDL_GPUDeviceWrapper::SDL_GPUDeviceWrapper(SDL_GPUDeviceWrapper&& other) :
		m_GPUPtr(other.m_GPUPtr)
	{
		other.m_GPUPtr = nullptr;
	}

	SDL_GPUDeviceWrapper& SDL_GPUDeviceWrapper::operator= (SDL_GPUDeviceWrapper&& other) {
		if (&other != this) {
			// Cleanup current GPU Device
			if ((bool)(*this)) {
				SDL_DestroyGPUDevice(m_GPUPtr);
			}

			// Obtain next GPU device
			m_GPUPtr = other.m_GPUPtr;
			other.m_GPUPtr = nullptr;
		}
		return *this;
	}


	SDL_GPUDevice* SDL_GPUDeviceWrapper::Get() const {
		return m_GPUPtr;
	}

	SDL_GPUDeviceWrapper::operator SDL_GPUDevice* const () {
		return m_GPUPtr;
	}

	SDL_GPUDeviceWrapper::operator bool const () {
		return m_GPUPtr != nullptr;
	}

}
