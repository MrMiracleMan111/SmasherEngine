#include <cmath>
#include <chrono>
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

    void DrawableComponent::SetEntity(Entity& pEntity) {
        IComponent::SetEntity(pEntity);
        m_TransformChanged = true;
    }

    DrawableComponent& DrawableComponent::SetDepth(float depth) {
        m_TransformChanged = true;
        m_Depth = depth;
        return *this;
    }

    DrawableComponent& DrawableComponent::SetColor(sf::Color color) {
        m_TransformChanged = true;
        m_Color = color;
        return *this;
    }

    DrawableComponent& DrawableComponent::SetShader(std::shared_ptr<ShaderResource> pShader) {
        m_TransformChanged = true;
        m_ShaderResource = pShader;
        return *this;
    }

    DrawableComponent& DrawableComponent::SetClipRect(sf::IntRect clipRect) {
        m_ClipChanged = true;
        m_ClipRect = clipRect;
        return *this;
    }

    DrawableComponent& DrawableComponent::SetClipRotation(Degrees angle) {
        m_ClipChanged = true;
        m_ClipRotation = angle;
        return *this;
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
        
        float initialX = 0.5f * m_ClipRect.width;
        float initialY = 0.5f * m_ClipRect.height;

        float left = m_ClipRect.left - initialX; // +std::copysign(1.0f, m_ClipRect.width) * -0.5f;
        float top = m_ClipRect.top - initialY; //+std::copysign(1.0f, m_ClipRect.height) * -0.5f;


        m_ClipTransform
            .translate(initialX / dimensions.x, initialY / dimensions.y) // Center the clip at 0,0
            .rotate((float)m_ClipRotation)
            .scale(m_ClipRect.width / dimensions.x, m_ClipRect.height / dimensions.y)
            .translate(left / dimensions.x, top / dimensions.y);

        m_ClipChanged = false;
        return m_ClipTransform;
    }

}