#include "Bossbismark.h"

BossBismark::BossBismark(std::shared_ptr<GltfInstance> model, btRigidBody* body)
    : Enemy(model, body,
        /* maxHealth        */ 600.0f,
        /* speed            */ 5.5f,
        /* detectionRadius  */ 60.0f,
        /* fovDegrees       */ 180.0f) {
        if (mBody) {
        btTransform trans;
        mBody->getMotionState()->getWorldTransform(trans);
        mSpawnPos = glm::vec3(trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z());
    }
}

int BossBismark::getAnimIndex(EnemyAnimSlot slot) const {
    switch (slot) {
    case EnemyAnimSlot::Idle:        return 4;     case EnemyAnimSlot::Move:        return 7;     case EnemyAnimSlot::Attack1:     return 1;     case EnemyAnimSlot::Attack2:     return 3;     case EnemyAnimSlot::Attack3:     return 5;     case EnemyAnimSlot::HitReaction: return 6;     case EnemyAnimSlot::Death:       return 2;     default:                         return -1;     }
}
bool BossBismark::shouldResetPosition() const {
    btTransform trans;
    mBody->getMotionState()->getWorldTransform(trans);
    float currentY = trans.getOrigin().y();

        return (currentY < 7.0f);
}
std::vector<EnemyAnimSlot> BossBismark::getAvailableAttackSlots() const {
    return { EnemyAnimSlot::Attack1, EnemyAnimSlot::Attack2, EnemyAnimSlot::Attack3 };
}

std::vector<HitWindow> BossBismark::getHitWindows(EnemyAnimSlot attackSlot) const {
    switch (attackSlot) {
    case EnemyAnimSlot::Attack1: 
        return { HitWindow{0.50f, 0.60f, false} };
    case EnemyAnimSlot::Attack2: 
        return { HitWindow{0.30f, 0.60f, false} };
    case EnemyAnimSlot::Attack3: 
        return { HitWindow{0.65f, 0.67f, false} };
    default:
        return { HitWindow{0.65f, 0.67f, false} };
    }
}

void BossBismark::onAttackPhysicsUpdate(float attackTimeAccumulator, float deltaTime, EnemyAnimSlot currentAttackAnim)
{
        if (currentAttackAnim != EnemyAnimSlot::Attack2) return;

    btTransform trans;
    mBody->getMotionState()->getWorldTransform(trans);

        btVector3 forwardDir = quatRotate(trans.getRotation(), btVector3(0, 0, 1));
    btVector3 backwardDir = -forwardDir;

    float moveSpeed = 0.0f;
    btVector3 targetDir = forwardDir;

        
    if (attackTimeAccumulator > (5.0f / 60.0f) && attackTimeAccumulator < (40.0f / 60.0f)) {
        moveSpeed = 10.0f;         targetDir = forwardDir;
    }
            else if (attackTimeAccumulator >= (40.0f / 60.0f) && attackTimeAccumulator < (60.0f / 60.0f)) {
        moveSpeed = 2.0f;
        targetDir = forwardDir;
    }


        btVector3 currentVel = mBody->getLinearVelocity();
    float accel = 30.0f * deltaTime;

        btVector3 targetVel(targetDir.x() * moveSpeed, currentVel.y(), targetDir.z() * moveSpeed);
    btVector3 finalVel = currentVel.lerp(targetVel, accel);

    mBody->setLinearVelocity(finalVel);
    mBody->setAngularVelocity(btVector3(0, 0, 0)); }
