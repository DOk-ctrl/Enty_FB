#define _USE_MATH_DEFINES
#include "AIStates.h"
#include "Enemy.h"
#include <iostream>
#include <cstdlib>
#include <algorithm>

void AIStateIdle::enter(Enemy* enemy) {
    enemy->setEngaged(false);

    auto settings = enemy->getModel()->getInstanceSettings();
    int clip = enemy->getAnimIndex(EnemyAnimSlot::Idle);

    settings.msAnimClip = clip;
    settings.msCrossBlendDestAnimClip = clip;     settings.msBlendingMode = blendMode::crossfade;
    settings.msAnimCrossBlendFactor = 1.0f;
    settings.msAnimTimePosition = 0.0f;
    settings.msPlayAnimation = true;
    settings.msAnimSpeed = 1.0f;

    enemy->getModel()->setInstanceSettings(settings);
    enemy->getModel()->checkForUpdates();
}

void AIStateIdle::update(Enemy* enemy, float deltaTime) {
    if (enemy->CanSeeTarget()) {
        enemy->changeState(std::make_unique<AIStateChase>());
    }
}

void AIStateChase::enter(Enemy* enemy) {
                enemy->setEngaged(true);

    auto settings = enemy->getModel()->getInstanceSettings();
    int clip = enemy->getAnimIndex(EnemyAnimSlot::Move);
    if (clip < 0) {
                        clip = enemy->getAnimIndex(EnemyAnimSlot::Idle);
    }

    settings.msAnimClip = clip;
    settings.msCrossBlendDestAnimClip = clip;
    settings.msBlendingMode = blendMode::crossfade;
    settings.msAnimCrossBlendFactor = 1.0f;
    settings.msAnimTimePosition = 0.0f;
    settings.msPlayAnimation = true;
    settings.msAnimSpeed = 1.5f; 
    enemy->getModel()->setInstanceSettings(settings);
    enemy->getModel()->checkForUpdates();
}

void AIStateChase::update(Enemy* enemy, float deltaTime) {
    btRigidBody* body = enemy->getBody();
    if (!body) return;

    if (!enemy->CanSeeTarget()) {
        enemy->changeState(std::make_unique<AIStateIdle>());
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

        if (distance < enemy->getAttackRange()) {
                if (enemy->canBlock() && (rand() % 100) < 25) {
            enemy->changeState(std::make_unique<AIStateBlock>());
        }
        else {
            enemy->changeState(std::make_unique<AIStateAttack>());
        }
        return;
    }

    if (distance > 0.1f) {
        dir = glm::normalize(dir);
        float accel = 10.0f * deltaTime;

        btVector3 targetVel(dir.x * enemy->getSpeed(), currentVel.y(), dir.z * enemy->getSpeed());
        body->setLinearVelocity(currentVel.lerp(targetVel, accel));

        float targetAngle = atan2(dir.x, dir.z);
        btVector3 currentForward = quatRotate(trans.getRotation(), btVector3(0, 0, 1));
        float currentAngle = atan2(currentForward.x(), currentForward.z());
        float angleDiff = targetAngle - currentAngle;

        while (angleDiff > M_PI) angleDiff -= 2.0f * M_PI;
        while (angleDiff < -M_PI) angleDiff += 2.0f * M_PI;

        body->setAngularVelocity(btVector3(0, angleDiff * 10.0f, 0));
    }
}

void AIStateAttack::enter(Enemy* enemy) {
    mActiveSlot = enemy->chooseAttackSlot();
    int clip = enemy->getAnimIndex(mActiveSlot);
    if (clip < 0) clip = enemy->getAnimIndex(EnemyAnimSlot::Attack1); 
    auto settings = enemy->getModel()->getInstanceSettings();
    settings.msAnimClip = clip;
    settings.msCrossBlendDestAnimClip = clip;
    settings.msBlendingMode = blendMode::crossfade;
    settings.msAnimCrossBlendFactor = 1.0f;

        settings.msAnimTimePosition = 0.0f;
    settings.msPlayAnimation = false;     settings.msAnimSpeed = 1.0f;

    enemy->getModel()->setInstanceSettings(settings);
    enemy->getModel()->checkForUpdates();

        btRigidBody* body = enemy->getBody();
    if (body) {
        body->setLinearVelocity(btVector3(0, body->getLinearVelocity().y(), 0));
        body->setAngularVelocity(btVector3(0, 0, 0));
    }

    mAttackTimer = 0.0f;
    mAttackDuration = enemy->getModel()->getAnimationEndTime(clip);
    if (mAttackDuration <= 0.01f) {
        mAttackDuration = 1.0f;     }

    mHitWindows = enemy->getHitWindows(mActiveSlot);
    for (auto& w : mHitWindows) w.consumed = false;
}

void AIStateAttack::update(Enemy* enemy, float deltaTime) {
        if (enemy->shouldResetPosition()) {
        enemy->resetToSpawn();
        return;
    }
    mAttackTimer += deltaTime; 
    auto settings = enemy->getModel()->getInstanceSettings();
    settings.msPlayAnimation = false;
    settings.msAnimTimePosition = mAttackTimer;     enemy->getModel()->setInstanceSettings(settings);
    enemy->getModel()->checkForUpdates();


    float t = (mAttackDuration > 0.0f) ? (mAttackTimer / mAttackDuration) : 0.0f;

        for (auto& window : mHitWindows) {
        if (window.consumed) continue;
        if (t >= window.startFrac && t <= window.endFrac) {
            btTransform trans;
            enemy->getBody()->getMotionState()->getWorldTransform(trans);
            glm::vec3 currentPos(trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z());

            glm::vec3 dirToTarget = enemy->getTargetPos() - currentPos;
            dirToTarget.y = 0.0f;
            float distance = glm::length(dirToTarget);

                                    float hitRange = enemy->getAttackRange() + 1.0f;

            if (distance <= hitRange) {
                enemy->setJustHitPlayer(true);                 window.consumed = true;
                std::cout << "[ENEMY] Удар прошёл!" << std::endl;
            }
        }
    
    }
    enemy->onAttackPhysicsUpdate(mAttackTimer, deltaTime, mActiveSlot);
        if (mAttackTimer >= mAttackDuration) {
                enemy->changeState(std::make_unique<AIStateChase>());
    }
}

void AIStateHitReaction::enter(Enemy* enemy) {
    int clip = enemy->getAnimIndex(EnemyAnimSlot::HitReaction);
    if (clip < 0) {
                        enemy->changeState(std::make_unique<AIStateChase>());
        return;
    }
        enemy->incrementStunCount();

    auto settings = enemy->getModel()->getInstanceSettings();
    settings.msAnimClip = clip;
    settings.msCrossBlendDestAnimClip = clip;
    settings.msBlendingMode = blendMode::crossfade;
    settings.msAnimCrossBlendFactor = 1.0f;

    settings.msAnimTimePosition = 0.0f;
    settings.msPlayAnimation = false;     settings.msAnimSpeed = 1.0f;

    enemy->getModel()->setInstanceSettings(settings);
    enemy->getModel()->checkForUpdates();

    btRigidBody* body = enemy->getBody();
    if (body) {
        body->setLinearVelocity(btVector3(0, body->getLinearVelocity().y(), 0));
        body->setAngularVelocity(btVector3(0, 0, 0));
    }

    mStunTimer = 0.0f;
    mStunDuration = enemy->getModel()->getAnimationEndTime(clip);
    if (mStunDuration <= 0.01f) {
        mStunDuration = 0.6f;     }
}

void AIStateHitReaction::update(Enemy* enemy, float deltaTime) {
    mStunTimer += deltaTime;

    auto settings = enemy->getModel()->getInstanceSettings();
    settings.msPlayAnimation = false;
    settings.msAnimTimePosition = mStunTimer;
    enemy->getModel()->setInstanceSettings(settings);
    enemy->getModel()->checkForUpdates();

    if (mStunTimer >= mStunDuration) {
        std::cout << "[AI] Враг пришел в себя после удара и продолжает бой!" << std::endl;
        enemy->changeState(std::make_unique<AIStateChase>());
    }
}

void AIStateDeath::enter(Enemy* enemy) {
    int clip = enemy->getAnimIndex(EnemyAnimSlot::Death);

    auto settings = enemy->getModel()->getInstanceSettings();
    settings.msAnimClip = clip;
    settings.msCrossBlendDestAnimClip = clip;
    settings.msBlendingMode = blendMode::crossfade;
    settings.msAnimCrossBlendFactor = 1.0f;

    settings.msAnimTimePosition = 0.0f;
    settings.msPlayAnimation = false;     settings.msAnimSpeed = 1.0f;

    enemy->getModel()->setInstanceSettings(settings);
    enemy->getModel()->checkForUpdates();

        btRigidBody* body = enemy->getBody();
    if (body) {
        body->setLinearVelocity(btVector3(0, body->getLinearVelocity().y(), 0));
        body->setAngularVelocity(btVector3(0, 0, 0));
        body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    }

    mDeathTimer = 0.0f;
    mDeathDuration = enemy->getModel()->getAnimationEndTime(clip);
    std::cout << "[AI] Враг мертв." << std::endl;
}

void AIStateDeath::update(Enemy* enemy, float deltaTime) {
        if (mDeathTimer < mDeathDuration) {
        mDeathTimer += deltaTime;

        auto settings = enemy->getModel()->getInstanceSettings();
        settings.msAnimTimePosition = std::min(mDeathTimer, mDeathDuration - 0.01f);

        enemy->getModel()->setInstanceSettings(settings);
        enemy->getModel()->checkForUpdates();
    }
    }

void AIStateBlock::enter(Enemy* enemy) {
    int clip = enemy->getAnimIndex(EnemyAnimSlot::Block);
    if (clip < 0) {
                enemy->changeState(std::make_unique<AIStateChase>());
        return;
    }

    auto settings = enemy->getModel()->getInstanceSettings();
    settings.msAnimClip = clip;
    settings.msCrossBlendDestAnimClip = clip;
    settings.msBlendingMode = blendMode::crossfade;
    settings.msAnimCrossBlendFactor = 1.0f;
    settings.msAnimTimePosition = 0.0f;
    settings.msPlayAnimation = true;     settings.msAnimSpeed = 1.0f;

    enemy->getModel()->setInstanceSettings(settings);
    enemy->getModel()->checkForUpdates();

    btRigidBody* body = enemy->getBody();
    if (body) {
        body->setLinearVelocity(btVector3(0, body->getLinearVelocity().y(), 0));
        body->setAngularVelocity(btVector3(0, 0, 0));
    }

    enemy->setBlocking(true);
    mBlockTimer = 0.0f;
    std::cout << "[AI] Босс встал в блок." << std::endl;
}

void AIStateBlock::update(Enemy* enemy, float deltaTime) {
    mBlockTimer += deltaTime;

    if (!enemy->CanSeeTarget() || mBlockTimer >= mBlockDuration) {
        enemy->changeState(std::make_unique<AIStateChase>());
    }
}

void AIStateBlock::exit(Enemy* enemy) {
    enemy->setBlocking(false);
}