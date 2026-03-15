#include "entt/entity/registry.hpp"
#include <GL/glew.h>
#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#elif defined(__linux__)
#include <GL/gl.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#endif
#include "Smasher/ErrorCodes.h"
#include "Smasher/ComponentSystems/EngineSystem.h"
#include "Smasher/Engine.h"


namespace Smasher {
	namespace EngineSystem {
		ErrorCode ClearWindow(entt::registry& registry) {
			if (!registry.ctx().contains<Context>()) {
				return ERROR_EngineNotInitialized;
			}

			auto& engine = registry.ctx().get<Context>().engineRef.get();
			return engine.ClearWindow();
		}

		ErrorCode DisplayWindow(entt::registry& registry) {
			if (!registry.ctx().contains<Context>()) {
				return ERROR_EngineNotInitialized;
			}

			auto& engine = registry.ctx().get<Context>().engineRef.get();
			ErrorCode code = engine.DisplayWindow();

			GLenum err;
			while ((err = glGetError()) != GL_NO_ERROR)
			{
				std::cout << "GL Error: \"" << gluErrorString(err) << "\" Code: " << err << std::endl;
			}
			return code;
		}
	}
}