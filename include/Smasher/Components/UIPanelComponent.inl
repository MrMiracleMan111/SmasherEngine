#pragma once
namespace Smasher {
	template<class T>
	UIPanelComponent& UIPanelComponent::SetOnHoverCallback(void (T::* method)(Events::MouseMoveEvent&), T* instance) {
		return SetOnHoverCallback(std::bind(method, instance, std::placeholders::_1));
	}

	template<class T>
	UIPanelComponent& UIPanelComponent::SetOnPressCallback(void (T::* method)(Events::MouseButtonEvent&), T* instance) {
		return SetOnPressCallback(std::bind(method, instance, std::placeholders::_1));
	}
}