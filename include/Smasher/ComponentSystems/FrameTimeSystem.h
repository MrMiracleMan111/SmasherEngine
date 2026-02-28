#include <chrono>
#include "entt/entity/registry.hpp"
#include "Smasher/Base.h"
#pragma once
#include "Smasher/ErrorCodes.h"

namespace Smasher {
	namespace FrameTimeSystem {
		struct SMASHER_API Context {
			std::chrono::time_point<std::chrono::system_clock> m_FrameStart;
			std::chrono::microseconds m_FrameTime; // Time spent on the most recent frame
			std::chrono::microseconds m_AvgFrameTime; // Average frame time over "m_SampleCount" number 
													  // of samples
			std::chrono::microseconds m_FrameTimeSum; // Sum of prior frame times in milliseconds
			const unsigned int m_MaxSampleCount = 5; // How many samples to take for frame time average

			unsigned int m_SampleCount = 0; // How many samples to take for frame time average
		};

		SMASHER_API const ErrorCode Initialize(entt::registry& registry);
		SMASHER_API const ErrorCode Teardown(entt::registry& registry);
		SMASHER_API const ErrorCode StartFrame(entt::registry& registry);
		SMASHER_API const ErrorCode EndFrame(entt::registry& registry);
	}
}