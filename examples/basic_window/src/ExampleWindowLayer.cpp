#include "ExampleWindowLayer.h"

ExampleWindowLayer::~ExampleWindowLayer()
{
	GetEngine().GetEventManager().Unsubscribe(m_KeyPressSubscription);
}

void ExampleWindowLayer::Init() {
	m_KeyPressSubscription = GetEngine().GetEventManager().Subscribe<Smasher::Events::KeyboardEvent>(&ExampleWindowLayer::OnKeyPress, this);
}

void ExampleWindowLayer::Reset() {

}

void ExampleWindowLayer::Render(sf::RenderWindow& window) {

}

void ExampleWindowLayer::OnKeyPress(Smasher::Events::KeyboardEvent& e) {
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