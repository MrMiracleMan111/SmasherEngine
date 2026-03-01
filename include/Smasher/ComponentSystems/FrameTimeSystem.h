#include <chrono>
#include "entt/entity/registry.hpp"
#include "Smasher/Base.h"
#pragma once
#include "Smasher/ErrorCodes.h"

namespace Smasher {
	namespace FrameTimeSystem {
		struct SMASHER_API Context {
			std::chrono::time_point<std::chrono::system_clock> frameStart;
			std::chrono::microseconds frameTime; // Time spent on the most recent frame
			std::chrono::microseconds avgFrameTime; // Average frame time over "sampleCount" number 
													  // of samples
			std::chrono::microseconds frameTimeSum; // Sum of prior frame times in milliseconds
			const unsigned int maxSampleCount = 5; // How many samples to take for frame time average

			unsigned int sampleCount = 0; // How many samples to take for frame time average
		};

		SMASHER_API const ErrorCode Initialize(entt::registry& registry);
		SMASHER_API const ErrorCode Teardown(entt::registry& registry);
		SMASHER_API const ErrorCode StartFrame(entt::registry& registry);
		SMASHER_API const ErrorCode EndFrame(entt::registry& registry);
	}
}