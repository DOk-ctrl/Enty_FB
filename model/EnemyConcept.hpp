#pragma once
#include <concepts>
#include <memory>
#include <glm/glm.hpp>

class btRigidBody;
class GltfInstance;

template<typename T>
concept EnemyConcept = requires(T& enemy, float dt, const glm::vec3 & pos, float damage) {
        typename T::AnimConfig;

        { enemy.update(dt, pos) } -> std::same_as<void>;
    { enemy.takeDamage(damage) } -> std::same_as<void>;
    { enemy.isDead() } -> std::convertible_to<bool>;
    { enemy.canSeeTarget() } -> std::convertible_to<bool>;

        { enemy.getModel() } -> std::same_as<std::shared_ptr<GltfInstance>>;
    { enemy.getBody() } -> std::same_as<btRigidBody*>;
    { enemy.getTargetPos() } -> std::convertible_to<glm::vec3>;
    { enemy.getSpeed() } -> std::convertible_to<float>;
};