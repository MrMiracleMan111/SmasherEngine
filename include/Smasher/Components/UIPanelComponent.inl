#pragma once
namespace Smasher {
	template<class T>
	UIPanelComponent& UIPanelComponent::SetOnHoverCallback(void (T:: *method)(Events::MouseMoveEvent&), T *pInstance) {
		return SetOnHoverCallback(std::bind(method, pInstance, std::placeholders::_1));
	}

	template<class T>
	UIPanelComponent& UIPanelComponent::SetOnPressCallback(void (T:: *method)(Events::MouseButtonEvent&), T *pInstance) {
		return SetOnPressCallback(std::bind(method, pInstance, std::placeholders::_1));
	}
}