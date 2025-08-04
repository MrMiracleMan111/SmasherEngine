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
        m_TransformPtr = &GetEntity().GetComponent<Transform2DComponent>().GetTransform();
    }
}