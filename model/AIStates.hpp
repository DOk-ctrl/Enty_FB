#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <memory>
#include <iostream>
#include <algorithm>
#include <glm/glm.hpp>
#include <btBulletDynamicsCommon.h>
#include <GltfInstance.h>

#include "EnemyConcept.hpp"

// Forward declarations для всех состояний
template<EnemyConcept T> class AIStateIdle;
template<EnemyConcept T> class AIStateChase;
template<EnemyConcept T> class AIStateAttack;
template<EnemyConcept T> class AIStateHitReaction;
template<EnemyConcept T> class AIStateDeath;

// ==========================================
// БАЗОВЫЙ ШАБЛОННЫЙ ИНТЕРФЕЙС
// ==========================================
template<typename T>
class AIState {
public:
    virtual ~AIState() = default;
    virtual void enter(T* enemy) = 0;
    virtual void update(T* enemy, float deltaTime) = 0;
};

// ==========================================
// 1. ЛОГИКА ОЖИДАНИЯ (IDLE)
// ==========================================
template<EnemyConcept T>
class AIStateIdle : public AIState<T> {
private:
    float mWaitTimer = 0.0f;
public:
    void enter(T* enemy) override {
        mWaitTimer = 0.0f;
        auto settings = enemy->getModel()->getInstanceSettings();

        settings.msAnimClip = T::AnimConfig::IDLE;
        settings.msCrossBlendDestAnimClip = T::AnimConfig::IDLE;
        settings.msBlendingMode = blendMode::crossfade;
        settings.msAnimCrossBlendFactor = 1.0f;
        settings.msAnimTimePosition = 0.0f;
        settings.msPlayAnimation = true;
        settings.msAnimSpeed = 1.0f;

        enemy->getModel()->setInstanceSettings(settings);
        enemy->getModel()->checkForUpdates();
    }

    void update(T* enemy, float deltaTime) override {
        if (enemy->canSeeTarget()) {
            enemy->changeState(std::make_unique<AIStateChase<T>>());
        }
    }
};

// ==========================================
// 2. ЛОГИКА ПОГОНИ (CHASE)
// ==========================================
template<EnemyConcept T>
class AIStateChase : public AIState<T> {
public:
    void enter(T* enemy) override {
        auto settings = enemy->getModel()->getInstanceSettings();

        settings.msAnimClip = T::AnimConfig::RUN;
        settings.msCrossBlendDestAnimClip = T::AnimConfig::RUN;
        settings.msBlendingMode = blendMode::crossfade;
        settings.msAnimCrossBlendFactor = 1.0f;
        settings.msAnimTimePosition = 0.0f;
        settings.msPlayAnimation = true;
        settings.msAnimSpeed = 1.5f;

        enemy->getModel()->setInstanceSettings(settings);
        enemy->getModel()->checkForUpdates();
    }

    void update(T* enemy, float deltaTime) override {
        btRigidBody* body = enemy->getBody();
        if (!body) return;

        if (!enemy->canSeeTarget()) {
            enemy->changeState(std::make_unique<AIStateIdle<T>>());
            return;
        }

        body->activate(true);

        btVector3 currentVel = body->getLinearVelocity();
        glm::vec3 target = enemy->getTargetPos();

        btTransform trans;
        body->getMotionState()->getWorldTransform(trans);
        glm::vec3 currentPos(trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z());

        glm::vec3 dir = target - currentPos;
        dir.y = 0.0f;

        float distance = glm::length(dir);

        // Если добежали вплотную (1.5 метра) - бьем!
        if (distance < 1.5f) {
            enemy->changeState(std::make_unique<AIStateAttack<T>>());
            return;
        }

        if (distance > 0.1f) {
            dir = glm::normalize(dir);
            float accel = 10.0f * deltaTime;

            btVector3 targetVel(dir.x * enemy->getSpeed(), currentVel.y(), dir.z * enemy->getSpeed());
            body->setLinearVelocity(currentVel.lerp(targetVel, accel));

            // Поворот физического тела
            float targetAngle = std::atan2(dir.x, dir.z);
            btVector3 currentForward = quatRotate(trans.getRotation(), btVector3(0, 0, 1));
            float currentAngle = std::atan2(currentForward.x(), currentForward.z());
            float angleDiff = targetAngle - currentAngle;

            while (angleDiff > M_PI) angleDiff -= 2.0f * static_cast<float>(M_PI);
            while (angleDiff < -M_PI) angleDiff += 2.0f * static_cast<float>(M_PI);

            body->setAngularVelocity(btVector3(0, angleDiff * 10.0f, 0));
        }
    }
};

// ==========================================
// 3. ЛОГИКА АТАКИ (ATTACK)
// ==========================================
template<EnemyConcept T>
class AIStateAttack : public AIState<T> {
private:
    float mAttackTimer = 0.0f;
    float mAttackDuration = 0.0f;
    bool mHasDealtDamage = false;
public:
    void enter(T* enemy) override {
        auto settings = enemy->getModel()->getInstanceSettings();

        settings.msAnimClip = T::AnimConfig::ATTACK;
        settings.msCrossBlendDestAnimClip = T::AnimConfig::ATTACK;
        settings.msBlendingMode = blendMode::crossfade;
        settings.msAnimCrossBlendFactor = 1.0f;
        settings.msAnimTimePosition = 0.0f;
        settings.msPlayAnimation = false; // Ручное управление кадрами
        settings.msAnimSpeed = 1.0f;

        enemy->getModel()->setInstanceSettings(settings);
        enemy->getModel()->checkForUpdates();

        // Тормозим тело при ударе
        btRigidBody* body = enemy->getBody();
        if (body) {
            body->setLinearVelocity(btVector3(0, body->getLinearVelocity().y(), 0));
            body->setAngularVelocity(btVector3(0, 0, 0));
        }

        mAttackTimer = 0.0f;
        mAttackDuration = enemy->getModel()->getAnimationEndTime(T::AnimConfig::ATTACK);
        if (mAttackDuration <= 0.01f) {
            mAttackDuration = 1.0f;
        }
        mHasDealtDamage = false;
    }

    void update(T* enemy, float deltaTime) override {
        mAttackTimer += deltaTime;

        auto settings = enemy->getModel()->getInstanceSettings();
        constexpr float FPS = 60.0f;
        constexpr float hitWindowStart = 75.0f / FPS;
        constexpr float hitWindowEnd = 85.0f / FPS;

        settings.msPlayAnimation = false;
        settings.msAnimTimePosition = mAttackTimer;
        enemy->getModel()->setInstanceSettings(settings);
        enemy->getModel()->checkForUpdates();

        // Проверка окна урона
        if (settings.msAnimTimePosition >= hitWindowStart &&
            settings.msAnimTimePosition <= hitWindowEnd &&
            !mHasDealtDamage) {

            btTransform trans;
            enemy->getBody()->getMotionState()->getWorldTransform(trans);
            glm::vec3 currentPos(trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z());

            glm::vec3 dirToTarget = enemy->getTargetPos() - currentPos;
            dirToTarget.y = 0.0f;
            float distance = glm::length(dirToTarget);

            constexpr float attackRange = 2.5f;

            if (distance <= attackRange) {
                enemy->setJustHitPlayer(true);
                mHasDealtDamage = true;
                std::cout << "[ENEMY] Симбиот попал по игроку!" << std::endl;
            }
        }

        if (mAttackTimer >= mAttackDuration) {
            enemy->changeState(std::make_unique<AIStateChase<T>>());
        }
    }
};

// ==========================================
// 4. ЛОГИКА ПОЛУЧЕНИЯ ХИТА (HIT REACTION)
// ==========================================
template<EnemyConcept T>
class AIStateHitReaction : public AIState<T> {
private:
    float mStunTimer = 0.0f;
    float mStunDuration = 0.0f;
public:
    void enter(T* enemy) override {
        auto settings = enemy->getModel()->getInstanceSettings();

        settings.msAnimClip = T::AnimConfig::HIT;
        settings.msCrossBlendDestAnimClip = T::AnimConfig::HIT;
        settings.msBlendingMode = blendMode::crossfade;
        settings.msAnimCrossBlendFactor = 1.0f;
        settings.msAnimTimePosition = 0.0f;
        settings.msPlayAnimation = false;
        settings.msAnimSpeed = 1.0f;

        enemy->getModel()->setInstanceSettings(settings);
        enemy->getModel()->checkForUpdates();

        btRigidBody* body = enemy->getBody();
        if (body) {
            body->setLinearVelocity(btVector3(0, body->getLinearVelocity().y(), 0));
            body->setAngularVelocity(btVector3(0, 0, 0));
        }

        mStunTimer = 0.0f;
        mStunDuration = enemy->getModel()->getAnimationEndTime(T::AnimConfig::HIT);
        if (mStunDuration <= 0.01f) {
            mStunDuration = 0.6f;
        }
    }

    void update(T* enemy, float deltaTime) override {
        mStunTimer += deltaTime;

        auto settings = enemy->getModel()->getInstanceSettings();
        settings.msPlayAnimation = false;
        settings.msAnimTimePosition = mStunTimer;
        enemy->getModel()->setInstanceSettings(settings);
        enemy->getModel()->checkForUpdates();

        if (mStunTimer >= mStunDuration) {
            if (enemy->isDead()) {
                enemy->changeState(std::make_unique<AIStateDeath<T>>());
            }
            else {
                enemy->changeState(std::make_unique<AIStateChase<T>>());
            }
        }
    }
};

// ==========================================
// 5. ЛОГИКА СМЕРТИ (DEATH)
// ==========================================
template<EnemyConcept T>
class AIStateDeath : public AIState<T> {
private:
    float mDeathTimer = 0.0f;
    float mDeathDuration = 0.0f;
public:
    void enter(T* enemy) override {
        auto settings = enemy->getModel()->getInstanceSettings();

        settings.msAnimClip = T::AnimConfig::DEATH;
        settings.msCrossBlendDestAnimClip = T::AnimConfig::DEATH;
        settings.msBlendingMode = blendMode::crossfade;
        settings.msAnimCrossBlendFactor = 1.0f;
        settings.msAnimTimePosition = 0.0f;
        settings.msPlayAnimation = false;
        settings.msAnimSpeed = 1.0f;

        enemy->getModel()->setInstanceSettings(settings);
        enemy->getModel()->checkForUpdates();

        btRigidBody* body = enemy->getBody();
        if (body) {
            body->setLinearVelocity(btVector3(0, body->getLinearVelocity().y(), 0));
            body->setAngularVelocity(btVector3(0, 0, 0));
            body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
        }

        mDeathTimer = 0.0f;
        mDeathDuration = enemy->getModel()->getAnimationEndTime(T::AnimConfig::DEATH);
        std::cout << "[AI] Симбиот мертв." << std::endl;
    }

    void update(T* enemy, float deltaTime) override {
        if (mDeathTimer < mDeathDuration) {
            mDeathTimer += deltaTime;

            auto settings = enemy->getModel()->getInstanceSettings();
            settings.msAnimTimePosition = std::min(mDeathTimer, mDeathDuration - 0.01f);

            enemy->getModel()->setInstanceSettings(settings);
            enemy->getModel()->checkForUpdates();
        }
    }
};