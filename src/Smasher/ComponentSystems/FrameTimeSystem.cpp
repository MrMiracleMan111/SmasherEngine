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

		const ErrorCode Teardown(entt::registry& registry) {
			return ERROR_NoError;
		}

		const ErrorCode StartFrame(entt::registry& registry) {
			if (!registry.ctx().contains<Context>()) {
				return ERROR_SystemNotInitialized;
			}

			Context& ctx = registry.ctx().get<Context>();
			ctx.frameStart = std::chrono::system_clock::now();
			return ERROR_NoError;
		}

		const ErrorCode EndFrame(entt::registry& registry) {
			if (!registry.ctx().contains<Context>()) {
				return ERROR_SystemNotInitialized;
			}

			Context& ctx = registry.ctx().get<Context>();

			std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
			ctx.frameTime = std::chrono::duration_cast<std::chrono::microseconds>(end - ctx.frameStart);
			ctx.frameTimeSum += ctx.frameTime;
			ctx.sampleCount++;

			if (ctx.sampleCount >= ctx.maxSampleCount) {
				ctx.avgFrameTime = ctx.frameTimeSum / ctx.maxSampleCount;
				ctx.sampleCount = 0;
				std::cout << "Frame Sum Time: " << (int)(ctx.frameTimeSum.count()) << "us \n";
				std::cout << "Avg Frame Time: " << (int)(ctx.avgFrameTime.count()) << "us \n";
				std::cout << "Avg Frame Time: " << (float)(ctx.avgFrameTime.count()) / 1000.f << "ms \n";
				ctx.frameTimeSum = std::chrono::microseconds::zero();
			}

			return ERROR_NoError;
		}
	}
}