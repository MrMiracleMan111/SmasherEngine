#pragma once
#include <SFML/Graphics.hpp>

namespace Smasher {
	struct EngineConfig {
		const inline static int WINDOW_WIDTH = 640;
		const inline static int WINDOW_HEIGHT = 420;
		const inline static std::string TITLE = "Smasher Engine Game";
		const inline static unsigned int TARGET_UPDATE_RATE = 120u;
		const inline static unsigned int TARGET_FRAMERATE = 60u;
		const inline static unsigned int DEFAULT_FONT_SIZE = 30u;
		const inline static sf::Color DEFAULT_FONT_COLOR{ sf::Color::White };
		const inline static Millisecond UPDATE_INTERVAL = Millisecond { (1000u / TARGET_UPDATE_RATE) };
		const inline static Millisecond RENDER_INTERVAL = Millisecond{ (1000u / TARGET_FRAMERATE) };
		const inline static sf::ContextSettings DEFAULT_SETTINGS {
			24, // depthBits  // Request a 24 bits depth buffer
			8,  // stencilBits   // Request a 8 bits stencil buffer
			2,  // antialiasingLevel   // Request 2 levels of antialiasing
			3,  // majorVersion 
			3   // minorVersion 
		};
		const inline static std::string DRAWABLE_COMPONENT_VERT_SHADER = "#version 330 core \n precision highp int; layout(location = 0) in vec2 aPos; layout(location = 1) in vec2 aTexCoord; layout(location = 2) in vec3 aWorldPos; layout(location = 3) in vec2 aWorldScale; layout(location = 4) in mat3 texMatrix; layout(location = 7) in uint aColorCode; layout(location = 8) in float aWorldRotation; out vec4 vertexColor; out vec2 texCoord; uniform mat4 ViewProjectionMatrix; uniform ivec2 textureSize; mat4 GetTransform(vec3 position, vec2 scale, float rotation) { float c = cos(rotation); float s = sin(rotation); return mat4( scale.x * c, scale.x * s, 0.0, 0.0, -scale.y * s, scale.y * c, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, position.x, position.y, position.z, 1.0 ); } void main() { mat4 model = GetTransform(aWorldPos, aWorldScale, aWorldRotation); vertexColor = vec4( float((aColorCode >> 0) & 255u) / 255.0f, float((aColorCode >> 8) & 255u) / 255.0f, float((aColorCode >> 16) & 255u) / 255.0f, float((aColorCode >> 24) & 255u) / 255.0f ); gl_Position = ViewProjectionMatrix * model * vec4(aPos, 0.0, 1.0); vec3 texTmp = texMatrix * vec3(aTexCoord, 1.0); texCoord = texTmp.xy; }";
		const inline static std::string DRAWABLE_COMPONENT_FRAG_SHADER = "#version 330 core \n precision mediump float; precision highp int; in vec4 vertexColor; in vec2 texCoord; out vec4 FragColor; uniform sampler2D texture; uniform bool translucentPass; void main() { vec4 pixel = texture2D(texture, texCoord); if (!translucentPass && (pixel.a <= 0.95f)) { discard; } if (translucentPass && (pixel.a > 0.95f)) { discard; } FragColor = vertexColor * pixel; }";
	};
}