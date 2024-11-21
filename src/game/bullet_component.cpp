#include "bullet_component.hpp"
#include <iostream>


BulletComponent::BulletComponent(SceneObject *sObj, float xSpeed, float gravityAccel, bool useGravity)
    : Component(sObj), m_xSpeed(xSpeed), m_gravityAccel(gravityAccel), m_useGravity(useGravity)
{}


void BulletComponent::__on_collision(ColliderComponent &collider, Transform &transform)
{
    m_sceneObj->active = false;
}

/// @brief Spawns a bullet using its own transform as direction
void BulletComponent::spawn(float initialSpeed)
{
    Transform& transform = m_sceneObj->transform;
    glm::vec3 f = transform.get_front_vector();
    m_velocity = f * initialSpeed;
    m_horVector = glm::normalize(glm::vec3(f.x, 0.0f, f.z));
    transform.update_transform();
}

/// @brief Spawns a bullet using another transform as direction/position
/// @param spawner Bullet spawner
void BulletComponent::spawn(const Transform& spawner, float initialSpeed, bool useGravity)
{
    Transform& transform = m_sceneObj->transform;
    transform.set_world_position(spawner.get_world_position());
    transform.set_world_rotation(spawner.get_world_rotation());
    transform.update_transform();
    
    m_useGravity = useGravity;

    glm::vec3 f = spawner.get_front_vector();
    m_velocity = f * initialSpeed;
    m_horVector = glm::normalize(glm::vec3(f.x, 0.0f, f.z));
}

void BulletComponent::update(float dt)
{
    Transform& transform = m_sceneObj->transform;
    if (m_useGravity)
        m_velocity += glm::vec3(0.0f, 1.0f, 0.0f) * m_gravityAccel * dt;

    glm::vec3 fVel = (m_velocity + m_horVector) * dt;
    transform.set_world_position(transform.get_world_position() + fVel);
}