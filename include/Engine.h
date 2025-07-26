#pragma once
#include <unordered_map>
#include <optional>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "Base.h"
#include "GameState.h"

namespace Smasher {
	class SMASHER_API Engine {
	public:
		Engine();
		Engine(int width, int height);
		~Engine() {};

		void Update(Millisecond delta);
		void Render(sf::RenderWindow& window);
		void AddState(uint64_t id, GameState& state);
		void Run();
		void Shutdown();
		GameState& GetState(uint64_t id);

	private:
		std::unordered_map<uint64_t, std::unique_ptr<GameState>> m_GameStateMap;
		sf::RenderWindow m_Window;
		bool running = true;
	};
}