#include "ExampleWindowGameState.h"

ExampleWindowGameState::~ExampleWindowGameState()
{
	GetEngine().GetEventManager().Unsubscribe(m_KeyPressSubscription);
}

void ExampleWindowGameState::Init() {
	m_KeyPressSubscription = GetEngine().GetEventManager().Subscribe<Smasher::Events::KeyboardEvent>(&ExampleWindowGameState::OnKeyPress, this);
}

void ExampleWindowGameState::Reset() {

}

void ExampleWindowGameState::Render(sf::RenderWindow& window) {

}

void ExampleWindowGameState::OnKeyPress(const Smasher::Events::KeyboardEvent& e) {
	std::string type = "NA";
	switch (e.Type) {
		case Smasher::Keyboard::KeyboardEventType::KEY_PRESS:
			type = "KEY PRESS";
		break;
		case Smasher::Keyboard::KeyboardEventType::KEY_RELEASE:
			type = "KEY RELEASE";
		break;
		case Smasher::Keyboard::KeyboardEventType::KEY_HOLD:
			type = "KEY HOLD";
		break;
	}
	std::cout << "Key Event: " << type << " Key Code: " << e.KeyCode << std::endl;
}