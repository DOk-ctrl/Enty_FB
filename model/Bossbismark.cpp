#include "Bossbismark.h"

BossBismark::BossBismark(std::shared_ptr<GltfInstance> model, btRigidBody* body)
    : Enemy(model, body,
        /* maxHealth        */ 600.0f,
        /* speed            */ 5.5f,
        /* detectionRadius  */ 60.0f,
        /* fovDegrees       */ 5.0f) {
    // Присваиваем унаследованной переменной mSpawnPos начальные координаты из Bullet Physics:
    if (mBody) {
        btTransform trans;
        mBody->getMotionState()->getWorldTransform(trans);
        mSpawnPos = glm::vec3(trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z());
    }
}

int BossBismark::getAnimIndex(EnemyAnimSlot slot) const {
    switch (slot) {
    case EnemyAnimSlot::Idle:        return 4; // Idle
    case EnemyAnimSlot::Move:        return 7; // Walk
    case EnemyAnimSlot::Attack1:     return 1; // deafault kick
    case EnemyAnimSlot::Attack2:     return 3; // Flying Kick
    case EnemyAnimSlot::Attack3:     return 5; // MMA Kick
    case EnemyAnimSlot::HitReaction: return 6; // Reaction
    case EnemyAnimSlot::Death:       return 2; // Dying
    default:                         return -1; // Block не поддерживается
    }
}
bool BossBismark::shouldResetPosition() const {
    btTransform trans;
    mBody->getMotionState()->getWorldTransform(trans);
    float currentY = trans.getOrigin().y();

    // Если Бисмарк заманили на лестницу/сбросили ниже 7.0
    return (currentY < 7.0f);
}
std::vector<EnemyAnimSlot> BossBismark::getAvailableAttackSlots() const {
    return { EnemyAnimSlot::Attack1, EnemyAnimSlot::Attack2, EnemyAnimSlot::Attack3 };
}

std::vector<HitWindow> BossBismark::getHitWindows(EnemyAnimSlot attackSlot) const {
    switch (attackSlot) {
    case EnemyAnimSlot::Attack1: // default kick — быстрый одиночный удар
        return { HitWindow{0.50f, 0.60f, false} };
    case EnemyAnimSlot::Attack2: // Flying Kick — прыжок, удар в воздухе позже
        return { HitWindow{0.30f, 0.60f, false} };
    case EnemyAnimSlot::Attack3: // MMA Kick — размашистый удар
        return { HitWindow{0.65f, 0.67f, false} };
    default:
        return { HitWindow{0.65f, 0.67f, false} };
    }
}

void BossBismark::onAttackPhysicsUpdate(float attackTimeAccumulator, float deltaTime, EnemyAnimSlot currentAttackAnim)
{
    // Нам нужна физика только для Fly kick
    if (currentAttackAnim != EnemyAnimSlot::Attack2) return;

    btTransform trans;
    mBody->getMotionState()->getWorldTransform(trans);

    // Вектор "Вперед" для модели Конвея
    btVector3 forwardDir = quatRotate(trans.getRotation(), btVector3(0, 0, 1));
    btVector3 backwardDir = -forwardDir;

    float moveSpeed = 0.0f;
    btVector3 targetDir = forwardDir;

    // --- ТАЙМЛАЙН ДВИЖЕНИЯ В КОМБО ---
    // Вся анимация длится примерно 250 кадров (4.16 сек)

    if (attackTimeAccumulator > (5.0f / 60.0f) && attackTimeAccumulator < (40.0f / 60.0f)) {
        moveSpeed = 10.0f; // Скорость отскока
        targetDir = forwardDir;
    }
    // 2. Рывок вперед (допустим, с 40 по 90 кадр, то есть 0.66с - 1.5с)
    // Делаем скорость 15.0f, чтобы он пробежал бОльшую дистанцию и перекрыл отскок
    else if (attackTimeAccumulator >= (40.0f / 60.0f) && attackTimeAccumulator < (60.0f / 60.0f)) {
        moveSpeed = 2.0f;
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
