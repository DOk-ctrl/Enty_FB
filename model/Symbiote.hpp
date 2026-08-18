/*
#pragma once
#include <memory>
#include <glm/glm.hpp>
#include "btBulletDynamicsCommon.h"
#include "GltfInstance.h"
#include "EnemyConcept.hpp"

// Объявляем базовый шаблон без концепта (простой typename T)
template<typename T> class AIState;

class Symbiote {
public:
    struct AnimConfig {
        static constexpr int IDLE = 2;
        static constexpr int RUN = 5;
        static constexpr int ATTACK = 3;
        static constexpr int HIT = 4;
        static constexpr int DEATH = 1;
    };

private:
    std::shared_ptr<GltfInstance> mModel;
    btRigidBody* mBody;

    std::unique_ptr<AIState<Symbiote>> mCurrentState;

    glm::vec3 mTargetPos{ 0.0f };
    float mSpeed = 4.0f;
    float mDetectionRadius = 32.0f;
    float mFieldOfViewDegrees = 70.0f;
    float mHealth = 100.0f;

    bool mJustHitPlayer = false;

public:
    Symbiote(std::shared_ptr<GltfInstance> model, btRigidBody* body);
    ~Symbiote() = default;

    void update(float deltaTime, const glm::vec3& playerPos) {
        mTargetPos = playerPos;
        if (mCurrentState) {
            mCurrentState->update(this, deltaTime);
        }
    }

    void changeState(std::unique_ptr<AIState<Symbiote>> newState) {
        mCurrentState = std::move(newState);
        if (mCurrentState) {
            mCurrentState->enter(this);
        }
    }

    void takeDamage(float amount);

    bool canSeeTarget() const {
        if (!mBody) return false;

        btVector3 myPos = mBody->getWorldTransform().getOrigin();
        glm::vec3 enemyPos(myPos.x(), myPos.y(), myPos.z());

        float dist = glm::distance(enemyPos, mTargetPos);
        if (dist > mDetectionRadius) return false;

        return true;
    }

    bool isDead() const { return mHealth <= 0.0f; }
    void setJustHitPlayer(bool hit) { mJustHitPlayer = hit; }
    bool consumeHitPlayerFlag() {
        bool temp = mJustHitPlayer;
        mJustHitPlayer = false;
        return temp;
    }

    btRigidBody* getBody() { return mBody; }
    std::shared_ptr<GltfInstance> getModel() { return mModel; }
    glm::vec3 getTargetPos() const { return mTargetPos; }
    float getSpeed() const { return mSpeed; }
};

// Проверяем концепт ТОЛЬКО когда класс Symbiote ПОЛНОСТЬЮ ОБЪЯВЛЕН
static_assert(EnemyConcept<Symbiote>, "Symbiote must satisfy EnemyConcept!");

#include "AIStates.hpp"

inline Symbiote::Symbiote(std::shared_ptr<GltfInstance> model, btRigidBody* body)
    : mModel(model), mBody(body)
{
    changeState(std::make_unique<AIStateIdle<Symbiote>>());
}

inline void Symbiote::takeDamage(float amount) {
    mHealth -= amount;
    changeState(std::make_unique<AIStateHitReaction<Symbiote>>());
}
*/