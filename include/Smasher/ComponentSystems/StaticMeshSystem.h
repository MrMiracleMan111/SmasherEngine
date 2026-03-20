#pragma once
#include <glm/glm.hpp>
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

namespace StaticMeshSystem {
	struct Component {
		int materialId;
		int meshId;
	};
}
