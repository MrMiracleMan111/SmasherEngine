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
#include "Smasher/Base.h"
#include "Smasher/ComponentSystems/CameraSystem.h"

// Physical Based Rendering System
// From this guide: https://www.3dgep.com/forward-plus/#Deferred_Shading
//  ========= RENDER PIPELINE ========= 
//
// 
//   OPQAUE STAGE
// 
//	 ++++BUFFERS++++
//   Depth/Stencil		(D24_UNORM_S8_UINT)
//	 Light Accumulation (R8G8B8A8_UNORM  Back Buffer)
//   Albedo				(R8G8B8A8_UNORM)
//   Specular			(R8G8B8A8_UNORM)
//   Normals			(R32G32B32A32_FLOAT)
//
//   ++++PASSES++++
//   G-Buffer Pass (fills all buffers)
//	 Transluscent Pass
//
namespace PBRSystem {
	struct Context {
		// G-Buffer Objects
		GLuint gFBO;
		GLuint gLighting;
		GLuint gAlbedo;
		GLuint gSpecular;
		GLuint gNormals;

		// Shaders
		GLuint fragmentShader;
		GLuint vertexShader;
		GLuint postProcessShader; // Fragment Shader
	};
}
