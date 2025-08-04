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

    void DrawableComponent::SetEntity(Entity& pEntity) {
        IComponent::SetEntity(pEntity);
        assert(GetEntity().HasComponent<Transform2DComponent>());
        m_TransformPtr = &GetEntity().GetComponent<Transform2DComponent>().GetTransform();
    }

    DrawableComponent& DrawableComponent::SetDepth(float depth) {
        m_Depth = depth;
        return *this;
    }

    DrawableComponent& DrawableComponent::SetColor(sf::Color color) {
        m_Color = color;
        return *this;
    }

    DrawableComponent& DrawableComponent::SetShader(std::shared_ptr<ShaderResource> pShader) {
        m_ShaderResource = pShader;
        return *this;
    }

    DrawableComponent& DrawableComponent::SetClipRect(sf::IntRect clipRect) {
        m_ClipChanged = true;
        m_ClipRect = clipRect;
        return *this;
    }

    const sf::Transform& DrawableComponent::GetTransformPtr() const {
        assert(m_TransformPtr != nullptr);
        return *m_TransformPtr;
    }


    const sf::Transform& DrawableComponent::GetClipTransform()
    {
        assert(m_TextureLoaded);
        if (!m_ClipChanged) {
            return m_ClipTransform;
        }

        m_ClipTransform = sf::Transform::Identity;
        sf::Texture& pTexture = m_TextureResource->GetTexture();
        sf::Vector2f dimensions = sf::Vector2f(pTexture.getSize().x, pTexture.getSize().y);
        float left = m_ClipRect.left + std::copysign(1.0f, m_ClipRect.width) * -0.5f;
        float top = m_ClipRect.top + std::copysign(1.0f, m_ClipRect.height) * -0.5f;

        m_ClipTransform.scale(m_ClipRect.width / dimensions.x, m_ClipRect.height / dimensions.y);
        m_ClipTransform.translate(left / dimensions.x, top / dimensions.y);

        m_ClipChanged = false;
        return m_ClipTransform;
    }

}