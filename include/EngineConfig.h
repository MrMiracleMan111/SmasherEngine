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

	};
}