#include <iostream>
#include <chrono>
#include "Smasher/ComponentSystems/FrameTimeSystem.h"

namespace Smasher {
	namespace FrameTimeSystem {
		const ErrorCode Initialize(entt::registry& registry) {
			if (registry.ctx().contains<Context>()) {
				return ERROR_SystemAlreadyInitialized;
			}

			registry.ctx().emplace<Context>();
			return ERROR_NoError;
		}

		const ErrorCode StartFrame(entt::registry& registry) {
			if (!registry.ctx().contains<Context>()) {
				return ERROR_SystemNotInitialized;
			}

			Context& ctx = registry.ctx().get<Context>();
			ctx.m_FrameStart = std::chrono::system_clock::now();
			return ERROR_NoError;
		}

		const ErrorCode EndFrame(entt::registry& registry) {
			if (!registry.ctx().contains<Context>()) {
				return ERROR_SystemNotInitialized;
			}

			Context& ctx = registry.ctx().get<Context>();

			std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
			ctx.m_FrameTime = std::chrono::duration_cast<std::chrono::microseconds>(end - ctx.m_FrameStart);
			ctx.m_FrameTimeSum += ctx.m_FrameTime;
			ctx.m_SampleCount++;

			if (ctx.m_SampleCount >= ctx.m_MaxSampleCount) {
				ctx.m_AvgFrameTime = ctx.m_FrameTimeSum / ctx.m_MaxSampleCount;
				ctx.m_SampleCount = 0;
				std::cout << "Frame Sum Time: " << (int)(ctx.m_FrameTimeSum.count()) << "us \n";
				std::cout << "Avg Frame Time: " << (int)(ctx.m_AvgFrameTime.count()) << "us \n";
				std::cout << "Avg Frame Time: " << (float)(ctx.m_AvgFrameTime.count()) / 1000.f << "ms \n";
				ctx.m_FrameTimeSum = std::chrono::microseconds::zero();
			}

			return ERROR_NoError;
		}
	}
}