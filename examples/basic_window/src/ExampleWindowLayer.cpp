#include "ExampleWindowLayer.h"

ExampleWindowLayer::~ExampleWindowLayer()
{
	m_KeyPressSubscription.Unsubscribe();
}

void ExampleWindowLayer::Init() {
	m_KeyPressSubscription = Subscribe<Smasher::Events::KeyboardEvent>(&ExampleWindowLayer::OnKeyPress, this);
}

void ExampleWindowLayer::Reset() {

}

void ExampleWindowLayer::Render(sf::RenderWindow &window) {

}

void ExampleWindowLayer::OnKeyPress(Smasher::Events::KeyboardEvent &event) {
	std::string type = "NA";
	switch (event.Type) {
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
	std::cout << "Key Event: " << type << " Key Code: " << event.KeyCode << std::endl;
}