#include "Base.h"
#include <iostream>
namespace Smasher {

#ifdef BENCHMARK
	namespace InternalTimers {
		std::chrono::microseconds SMASHER_TimeAccumulator{}; // Used for time checking in benchmarks
		std::chrono::microseconds SMASHER_TimeSampleSum{}; // Used for time checking in benchmarks
		size_t SMASHER_TimeSampleCount{}; // Used for time checking in benchmarks
	}
#endif

	// Column major order Copy
	template<>
	void Matrix<3, 3>::CopyMatrix(const sf::Transform& transform) {
		const float* tmp = transform.getMatrix();
		for (int i = 0; i < 3; i++) {              // Row
			for (int j = 0; j < 2; j++) {          // Column
				array[i * 3 + j] = tmp[i * 4 + j];
			}
		}

		// Copy translation
		array[2 * 3 + 0] = tmp[3 * 4 + 0];
		array[2 * 3 + 1] = tmp[3 * 4 + 1];

		// Last Row is [0 0 1] to satify homogenous coordinates
		array[0 * 3 + 2] = 0.0f;
		array[1 * 3 + 2] = 0.0f;
		array[2 * 3 + 2] = 1.0f;

	}

	template<>
	void Matrix<4, 4>::CopyMatrix(const sf::Transform& transform) {
		CopyMatrix(transform.getMatrix(), 4 * 4, array);
	}

}
