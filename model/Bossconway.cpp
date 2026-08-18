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
    case EnemyAnimSlot::Idle:        return 4; // Idle
    case EnemyAnimSlot::Move:        return 8; //  ходьба 
    case EnemyAnimSlot::Attack1:     return 5; // punch
    case EnemyAnimSlot::Attack2:     return 7; // Uppercut
    case EnemyAnimSlot::Attack3:     return 2; // Combo
    case EnemyAnimSlot::Block:       return 1; // Block
    case EnemyAnimSlot::Death:       return 3; // Death
    case EnemyAnimSlot::HitReaction: return -1; // нет клипа — canBeStaggered() == false
    default:                         return -1;
    }
}

std::vector<EnemyAnimSlot> BossConway::getAvailableAttackSlots() const {
    return { EnemyAnimSlot::Attack1, EnemyAnimSlot::Attack2, EnemyAnimSlot::Attack3 };
}

std::vector<HitWindow> BossConway::getHitWindows(EnemyAnimSlot attackSlot) const {
    switch (attackSlot) {
    case EnemyAnimSlot::Attack1: // punch (64-71)
        return { HitWindow{0.52f, 0.58f, false} };
    case EnemyAnimSlot::Attack2: // uppercut (68-71)
        return { HitWindow{0.32f , 0.33f , false} };
    case EnemyAnimSlot::Attack3: // combo (4 окна!)
        return {
            HitWindow{0.28f, 0.3f, false}, // 1.66s - 1.75s
            HitWindow{0.49f, 0.51f, false}, // 2.86s - 3.00s
            HitWindow{0.59f, 0.60f, false}, // 3.45s - 3.56s
            HitWindow{0.67f, 0.68f, false}  // 3.93s - 4.00s
        };
    default:
        return { HitWindow{0.55f, 0.70f, false} };
    }
}

void BossConway::onAttackPhysicsUpdate(float attackTimeAccumulator, float deltaTime, EnemyAnimSlot currentAttackAnim)
{
    // Нам нужна физика только для комбо
    if (currentAttackAnim != EnemyAnimSlot::Attack3) return;

    btTransform trans;
    mBody->getMotionState()->getWorldTransform(trans);

    // Вектор "Вперед" для модели Конвея
    btVector3 forwardDir = quatRotate(trans.getRotation(), btVector3(0, 0, 1));
    btVector3 backwardDir = -forwardDir;

    float moveSpeed = 0.0f;
    btVector3 targetDir = forwardDir;

    // --- ТАЙМЛАЙН ДВИЖЕНИЯ В КОМБО ---
    // Вся анимация длится примерно 250 кадров (4.16 сек)

    if (attackTimeAccumulator > (90.0f / 60.0f) && attackTimeAccumulator < (130.0f / 60.0f)) {
        moveSpeed = 5.0f; // Скорость отскока
        targetDir = backwardDir;
    }
    // 2. Рывок вперед (допустим, с 40 по 90 кадр, то есть 0.66с - 1.5с)
    // Делаем скорость 15.0f, чтобы он пробежал бОльшую дистанцию и перекрыл отскок
    else if (attackTimeAccumulator >= (150.0f / 60.0f) && attackTimeAccumulator < (220.0f / 60.0f)) {
        moveSpeed = 9.0f;
        targetDir = forwardDir;
    }
    

    // Применяем физику (похоже на то, что ты делал для Энти)
    btVector3 currentVel = mBody->getLinearVelocity();
    float accel = 30.0f * deltaTime;

    // Меняем X и Z, оставляем Y как есть (гравитация)
    btVector3 targetVel(targetDir.x() * moveSpeed, currentVel.y(), targetDir.z() * moveSpeed);
    btVector3 finalVel = currentVel.lerp(targetVel, accel);

    mBody->setLinearVelocity(finalVel);
    mBody->setAngularVelocity(btVector3(0, 0, 0)); // Блокируем вращение, чтобы Конвей не завалился
}
