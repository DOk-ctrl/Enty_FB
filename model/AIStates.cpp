#define _USE_MATH_DEFINES
#include "AIStates.h"
#include "Enemy.h"
#include <iostream>

// ИНДЕКСЫ АНИМАЦИЙ Symbiote
const int ENEMY_ANIM_IDLE = 2;
const int ENEMY_ANIM_RUN = 5;
const int ENEMY_ANIM_ATTACK = 3;

// ==========================================
// 1. ЛОГИКА ОЖИДАНИЯ (IDLE)
// ==========================================
void AIStateIdle::enter(Enemy* enemy) {
    auto settings = enemy->getModel()->getInstanceSettings();

    settings.msAnimClip = ENEMY_ANIM_IDLE;
    settings.msCrossBlendDestAnimClip = ENEMY_ANIM_IDLE; // Обязательно для твоего движка
    settings.msBlendingMode = blendMode::crossfade;      // Включаем смешивание
    settings.msAnimCrossBlendFactor = 1.0f;
    settings.msAnimTimePosition = 0.0f;                  // Сброс таймера анимации
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

// ==========================================
// 2. ЛОГИКА ПОГОНИ (CHASE)
// ==========================================
void AIStateChase::enter(Enemy* enemy) {
    auto settings = enemy->getModel()->getInstanceSettings();

    settings.msAnimClip = ENEMY_ANIM_RUN;
    settings.msCrossBlendDestAnimClip = ENEMY_ANIM_RUN;
    settings.msBlendingMode = blendMode::crossfade;
    settings.msAnimCrossBlendFactor = 1.0f;
    settings.msAnimTimePosition = 0.0f;
    settings.msPlayAnimation = true;
    settings.msAnimSpeed = 1.5f; // Можно чуть ускорить ходьбу, чтобы выглядело как бег

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

    // Если добежали вплотную (полтора метра) - бьем!
    if (distance < 1.5f) {
        enemy->changeState(std::make_unique<AIStateAttack>());
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

// ==========================================
// 3. ЛОГИКА АТАКИ (ATTACK)
// ==========================================
void AIStateAttack::enter(Enemy* enemy) {
    auto settings = enemy->getModel()->getInstanceSettings();

    settings.msAnimClip = ENEMY_ANIM_ATTACK;
    settings.msCrossBlendDestAnimClip = ENEMY_ANIM_ATTACK;
    settings.msBlendingMode = blendMode::crossfade;
    settings.msAnimCrossBlendFactor = 1.0f;
    settings.msAnimTimePosition = 0.0f; // Удар всегда начинается с первого кадра
    settings.msPlayAnimation = true;
    settings.msAnimSpeed = 1.0f;

    enemy->getModel()->setInstanceSettings(settings);
    enemy->getModel()->checkForUpdates();

    // ЖЕСТКО ТОРМОЗИМ ВРАГА
    btRigidBody* body = enemy->getBody();
    if (body) {
        body->setLinearVelocity(btVector3(0, body->getLinearVelocity().y(), 0));
        body->setAngularVelocity(btVector3(0, 0, 0));
    }

    // --- ДИНАМИЧЕСКИЙ ТАЙМЕР АНИМАЦИИ ---
    // Спрашиваем у движка реальную длину клипа в секундах
    float clipDuration = enemy->getModel()->getAnimationEndTime(ENEMY_ANIM_ATTACK);

    // Если по какой-то причине движок вернул 0 (ошибка загрузки), ставим дефолт
    if (clipDuration <= 0.01f) {
        clipDuration = 1.5f;
    }

    // Таймер теперь равен реальной длине анимации!
    mAttackTimer = clipDuration;
}

void AIStateAttack::update(Enemy* enemy, float deltaTime) {
    mAttackTimer -= deltaTime;

    // Ждем, пока анимация проиграется полностью до последнего кадра
    if (mAttackTimer <= 0.0f) {
        enemy->changeState(std::make_unique<AIStateIdle>());
    }
}
