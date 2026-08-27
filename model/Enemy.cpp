#include "Enemy.h"
#include "AIStates.h"
#include <iostream>
#include <cstdlib>

Enemy::Enemy(std::shared_ptr<GltfInstance> model, btRigidBody* body,
    float maxHealth, float speed, float detectionRadius, float fovDegrees)
    : mModel(model), mBody(body),
    mSpeed(speed), mDetectionRadius(detectionRadius), mFieldOfViewDegrees(fovDegrees),
    mHealth(maxHealth), mMaxHealth(maxHealth) {
        changeState(std::make_unique<AIStateIdle>());
}

Enemy::~Enemy() {}

void Enemy::playmusic()
{

}

void Enemy::resetToSpawn()
{
    if (!mBody) return;

    btTransform initialTransform;
    initialTransform.setIdentity();
    initialTransform.setOrigin(btVector3(mSpawnPos.x, mSpawnPos.y, mSpawnPos.z));

        mBody->setWorldTransform(initialTransform);
    if (mBody->getMotionState()) {
        mBody->getMotionState()->setWorldTransform(initialTransform);
    }

        mBody->setLinearVelocity(btVector3(0, 0, 0));
    mBody->setAngularVelocity(btVector3(0, 0, 0));
    mBody->clearForces();

        changeState(std::make_unique<AIStateIdle>());

    std::cout << "[ENEMY] Враг сброшен на спавн!" << std::endl;
}

void Enemy::update(float deltaTime, const glm::vec3& playerPos) {
    mTargetPos = playerPos; 
    if (mCurrentState) {
        mCurrentState->update(this, deltaTime);     }
}

void Enemy::changeState(std::unique_ptr<AIState> newState) {
    if (mCurrentState) {
        mCurrentState->exit(this);
    }
    mCurrentState = std::move(newState);
    if (mCurrentState) {
        mCurrentState->enter(this);
    }
}

void Enemy::setJustHitPlayer(bool hit) {
    mJustHitPlayer = hit;
}

bool Enemy::consumeHitPlayerFlag() {
    if (mJustHitPlayer) {
        mJustHitPlayer = false;
        return true;
    }
    return false;
}

bool Enemy::CanSeeTarget() const {
    if (!mBody) return false;

    btTransform trans;
    mBody->getMotionState()->getWorldTransform(trans);
    glm::vec3 currentPos(trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z());

    glm::vec3 dirToTarget = mTargetPos - currentPos;
    dirToTarget.y = 0.0f;
    float distance = glm::length(dirToTarget);

    if (distance > mDetectionRadius) {
        return false;
    }

    if (distance > 0.1f) {
        dirToTarget = glm::normalize(dirToTarget);

        btVector3 btForward = quatRotate(trans.getRotation(), btVector3(0, 0, 1));
        glm::vec3 forward(btForward.x(), 0.0f, btForward.z());

        if (glm::length(forward) > 0.001f) {
            forward = glm::normalize(forward);
        }
        else {
            forward = glm::vec3(0.0f, 0.0f, 1.0f);
        }

        float dotProduct = glm::dot(forward, dirToTarget);
        float targetCos = glm::cos(glm::radians(mFieldOfViewDegrees));

        if (dotProduct < targetCos) {
            return false;
        }
    }

    return true;
}

void Enemy::TakeDamage(float amount) {
    if (isDead()) return; 
    float finalDamage = amount;
    if (isBlocking()) {
        finalDamage *= getBlockDamageMultiplier();
        std::cout << "[ENEMY] Атака заблокирована! Прошло только " << finalDamage << " урона." << std::endl;
    }

    mHealth -= finalDamage;
    std::cout << "[ENEMY] Получено " << finalDamage << " урона! Осталось ХП: " << mHealth << std::endl;

    if (mHealth <= 0.0f) {
        changeState(std::make_unique<AIStateDeath>());         return;
    }

    if (canBeStaggered()) {
        changeState(std::make_unique<AIStateHitReaction>());     }
        }

int Enemy::getAnimIndex(EnemyAnimSlot slot) const {
        switch (slot) {
    case EnemyAnimSlot::Idle:        return 2;
    case EnemyAnimSlot::Move:        return 5;
    case EnemyAnimSlot::Attack1:     return 3;
    case EnemyAnimSlot::HitReaction: return 4;
    case EnemyAnimSlot::Death:       return 1;
    default:                         return -1;     }
}

std::vector<EnemyAnimSlot> Enemy::getAvailableAttackSlots() const {
    return { EnemyAnimSlot::Attack1 };
}

EnemyAnimSlot Enemy::chooseAttackSlot() const {
    auto slots = getAvailableAttackSlots();
    if (slots.empty()) return EnemyAnimSlot::Attack1;
    int idx = rand() % static_cast<int>(slots.size());
    return slots[idx];
}

std::vector<HitWindow> Enemy::getHitWindows(EnemyAnimSlot /*attackSlot*/) const {
            return { HitWindow{0.55f, 0.70f, false} };
}