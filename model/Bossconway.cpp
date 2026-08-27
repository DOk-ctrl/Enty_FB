#include "BossConway.h"

BossConway::BossConway(std::shared_ptr<GltfInstance> model, btRigidBody* body)
    : Enemy(model, body,
        /* maxHealth        */ 800.0f,
        /* speed            */ 6.8f,
        /* detectionRadius  */ 40.0f,
        /* fovDegrees       */ 180.0f) {
}

int BossConway::getAnimIndex(EnemyAnimSlot slot) const {
    switch (slot) {
    case EnemyAnimSlot::Idle:        return 4;     case EnemyAnimSlot::Move:        return 8;     case EnemyAnimSlot::Attack1:     return 5;     case EnemyAnimSlot::Attack2:     return 7;     case EnemyAnimSlot::Attack3:     return 2;     case EnemyAnimSlot::Block:       return 1;     case EnemyAnimSlot::Death:       return 3;     case EnemyAnimSlot::HitReaction: return -1;     default:                         return -1;
    }
}

std::vector<EnemyAnimSlot> BossConway::getAvailableAttackSlots() const {
    return { EnemyAnimSlot::Attack1, EnemyAnimSlot::Attack2, EnemyAnimSlot::Attack3 };
}

std::vector<HitWindow> BossConway::getHitWindows(EnemyAnimSlot attackSlot) const {
    switch (attackSlot) {
    case EnemyAnimSlot::Attack1:         return { HitWindow{0.52f, 0.58f, false} };
    case EnemyAnimSlot::Attack2:         return { HitWindow{0.32f , 0.33f , false} };
    case EnemyAnimSlot::Attack3:         return {
            HitWindow{0.28f, 0.3f, false},             HitWindow{0.49f, 0.51f, false},             HitWindow{0.59f, 0.60f, false},             HitWindow{0.67f, 0.68f, false}          };
    default:
        return { HitWindow{0.55f, 0.70f, false} };
    }
}

void BossConway::onAttackPhysicsUpdate(float attackTimeAccumulator, float deltaTime, EnemyAnimSlot currentAttackAnim)
{
        if (currentAttackAnim != EnemyAnimSlot::Attack3) return;

    btTransform trans;
    mBody->getMotionState()->getWorldTransform(trans);

        btVector3 forwardDir = quatRotate(trans.getRotation(), btVector3(0, 0, 1));
    btVector3 backwardDir = -forwardDir;

    float moveSpeed = 0.0f;
    btVector3 targetDir = forwardDir;

        
    if (attackTimeAccumulator > (90.0f / 60.0f) && attackTimeAccumulator < (130.0f / 60.0f)) {
        moveSpeed = 5.0f;         targetDir = backwardDir;
    }
            else if (attackTimeAccumulator >= (150.0f / 60.0f) && attackTimeAccumulator < (220.0f / 60.0f)) {
        moveSpeed = 9.0f;
        targetDir = forwardDir;
    }
    

        btVector3 currentVel = mBody->getLinearVelocity();
    float accel = 30.0f * deltaTime;

        btVector3 targetVel(targetDir.x() * moveSpeed, currentVel.y(), targetDir.z() * moveSpeed);
    btVector3 finalVel = currentVel.lerp(targetVel, accel);

    mBody->setLinearVelocity(finalVel);
    mBody->setAngularVelocity(btVector3(0, 0, 0)); }
