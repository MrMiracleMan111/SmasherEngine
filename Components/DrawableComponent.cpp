#include <cmath>
#include "DrawableComponent.h"
#include "IComponent.h"
#include "Entity.h"
#include "Components/Transform2DComponent.h"
#include "ComponentManagers/DrawableComponentManager.h"

namespace Smasher {
    DrawableComponent::~DrawableComponent() {
        auto& rCompManager = static_cast<DrawableComponentManager&>(GetManager());
        rCompManager.OnComponentDelete(*this);
    }

    DrawableComponent& DrawableComponent::PushToGPU() {
        auto& rManager = static_cast<DrawableComponentManager&>(GetManager());
        rManager.OnComponentChangeData(*this);
        return *this;
    }

    sf::Transform DrawableComponent::GetClipTransform()
    {
        sf::Transform transform;
        float left = m_ClipRect.left + std::copysign(1.0f, m_ClipRect.width) * -0.5f;
        float top = m_ClipRect.top + std::copysign(1.0f, m_ClipRect.height) * -0.5f;

        transform.scale(m_ClipRect.width , m_ClipRect.height);
        transform.translate(left, top);

        return transform;
    }

    void DrawableComponent::SetEntity(Entity& pEntity) {
        IComponent::SetEntity(pEntity);
        assert(GetEntity().HasComponent<Transform2DComponent>());
        m_TransformRef = &GetEntity().GetComponent<Transform2DComponent>().GetTransform();
    }

    void DrawableComponent::applyCurrentView(sf::RenderTarget& target) const
    {
        // Set the viewport
        sf::IntRect viewport = getViewport(target, target.getView());
        int top = target.getSize().y - (viewport.top + viewport.height);
        (glViewport(viewport.left, top, viewport.width, viewport.height));

        // Set the projection matrix
        (glMatrixMode(GL_PROJECTION));
        (glLoadMatrixf(target.getView().getTransform().getMatrix()));

        // Go back to model-view mode
        (glMatrixMode(GL_MODELVIEW));
    }

    sf::IntRect DrawableComponent::getViewport(sf::RenderTarget& target, const sf::View& view) const
    {
        float width = static_cast<float>(target.getSize().x);
        float height = static_cast<float>(target.getSize().y);
        const sf::FloatRect& viewport = view.getViewport();

        return sf::IntRect(
            static_cast<int>(0.5f + width * viewport.left),
            static_cast<int>(0.5f + height * viewport.top),
            static_cast<int>(width * viewport.width),
            static_cast<int>(height * viewport.height));
    }
}