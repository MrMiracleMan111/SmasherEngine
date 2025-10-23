#include <numbers>
#include <iostream>
#include <cmath>
#include "Smasher/Base.h"
#include "SerializeLayer.h"
#include "Smasher/Layer.h"
#include "Manifest.h"
#include "Smasher/ComponentManagers/DrawableComponentManager.h"
#include "SavedComponent.h"
#include "BallComponent.h"

void SerializeLayer::Init()
{
	// Open save file
	std::fstream saveFile("SaveFile.bin", std::ios_base::in | std::ios_base::out | std::ios_base::app | std::ios_base::binary);

	if (!saveFile) {
		std::cerr << "Couldn't open SaveFile.bin" << std::endl;
		throw std::runtime_error("Couldn't open SaveFile.bin");
	}

	const size_t DIMENSION = 100;
	const size_t SPACING = 5;
	for (size_t i = 0; i < DIMENSION; i++) {
		for (size_t j = 0; j < DIMENSION; j++) {
			Smasher::Entity& ball = SpawnBouncingBall(sf::Vector2i(10, 10));
			ball.GetComponent<Smasher::DrawableComponent>()
				.SetPosition((float)(i * SPACING), (float)(j * SPACING))
				.SetTextureAsset<Smasher::Resources::Textures::small_art>({});
			ball.GetComponent<BallComponent>()
				.SetVelocity(ball.GetComponent<BallComponent>().GetVelocity() * 0.01f);
			ball.AddComponent<SavedComponent>();
			m_Balls.push_back(ball);
		}
	}

	size_t count = 0;
	Smasher::InputArchive archive(saveFile);
	while (saveFile.good() && saveFile.peek() != std::ifstream::traits_type::eof() && count < m_Balls.size()) {
		Smasher::Entity& ball = m_Balls[count];
		ball.GetComponent<SavedComponent>()
			.Deserialize(archive);
		count++;
	}
	std::cout << "Updated " << count << " component(s) from save file" << std::endl;

	saveFile.close();
	m_OnCloseSubscription = Subscribe<Smasher::Events::WindowCloseEvent>(&SerializeLayer::OnClose, this);
}

Smasher::Entity& SerializeLayer::SpawnBouncingBall(sf::Vector2i position)
{
	const float toRadian = (float)(180.0 / std::numbers::pi);
	int minSpeed = 100;
	int speedVariance = 100;
	float angle = (float)(rand() % 360);
	float speed = (float)(rand() % speedVariance + minSpeed);
	float tmpX = std::cos(angle * toRadian) * speed;
	float tmpY = std::sin(angle * toRadian) * speed;
	float depth = (float)(rand() % 100) / 100.0f;
	Smasher::Entity& image = AddEntity<Smasher::Entity>();
	image.AddComponent<Smasher::DrawableComponent>()
		.SetPosition(sf::Vector2f((float)position.x, (float)position.y))
		.SetScale(sf::Vector2f(20.0f, 20.0f))
		.SetDepth(depth)
		.GetEntity()
		.AddComponent<BallComponent>()
		.SetVelocity(sf::Vector2f(tmpX, tmpY));
	return image;
}

void SerializeLayer::OnClose(Smasher::Events::WindowCloseEvent& e)
{
	// Open save file
	std::fstream saveFile("SaveFile.bin", std::ios_base::out | std::ios_base::binary);

	Smasher::OutputArchive archive(saveFile);

	// Save all ball positions
	for (auto& rEntity : m_Balls) {
		rEntity.get().GetComponent<SavedComponent>()
			.Serialize(archive);
	}

	std::cout << "Saved " << m_Balls.size() << " component(s) to save file" << std::endl;


	saveFile.close();
}
