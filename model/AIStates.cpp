#define _USE_MATH_DEFINES
#include "AIStates.h"
#include "Enemy.h"
#include <iostream>
#include <cstdlib>
#include <algorithm>

// ==========================================
// 1. ЛОГИКА ОЖИДАНИЯ (IDLE)
// ==========================================
void AIStateIdle::enter(Enemy* enemy) {
    enemy->setEngaged(false);

    auto settings = enemy->getModel()->getInstanceSettings();
    int clip = enemy->getAnimIndex(EnemyAnimSlot::Idle);

    settings.msAnimClip = clip;
    settings.msCrossBlendDestAnimClip = clip; // Обязательно для твоего движка
    settings.msBlendingMode = blendMode::crossfade;
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

// ==========================================
// 2. ЛОГИКА ПОГОНИ (CHASE)
// ==========================================
void AIStateChase::enter(Enemy* enemy) {
    // Первый вход в Chase = игрок обнаружен -> помечаем как "вовлечён в бой".
    // Дальше флаг не сбрасывается (кроме смерти, которую отдельно фильтруем при отрисовке
    // health bar) — поэтому полоса не мигает, если босс на секунду теряет вас из FOV.
    enemy->setEngaged(true);

    auto settings = enemy->getModel()->getInstanceSettings();
    int clip = enemy->getAnimIndex(EnemyAnimSlot::Move);
    if (clip < 0) {
        // У врага нет отдельного клипа ходьбы/бега (напр. Conway) — играем Idle,
        // физика всё равно тащит тело к игроку.
        clip = enemy->getAnimIndex(EnemyAnimSlot::Idle);
    }

    settings.msAnimClip = clip;
    settings.msCrossBlendDestAnimClip = clip;
    settings.msBlendingMode = blendMode::crossfade;
    settings.msAnimCrossBlendFactor = 1.0f;
    settings.msAnimTimePosition = 0.0f;
    settings.msPlayAnimation = true;
    settings.msAnimSpeed = 1.5f; // Можно чуть ускорить, чтобы выглядело как бег

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

    // Дистанция атаки теперь берётся из самого врага (у боссов может отличаться)
    if (distance < enemy->getAttackRange()) {
        // Боссы, умеющие блокировать, иногда блокируют вместо удара (25% шанс)
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

// ==========================================
// 3. ЛОГИКА АТАКИ (ATTACK) — генерик: слот выбирается через enemy,
//    поддерживает несколько окон урона за одну анимацию (Combo).
// ==========================================
void AIStateAttack::enter(Enemy* enemy) {
    mActiveSlot = enemy->chooseAttackSlot();
    int clip = enemy->getAnimIndex(mActiveSlot);
    if (clip < 0) clip = enemy->getAnimIndex(EnemyAnimSlot::Attack1); // страховка

    auto settings = enemy->getModel()->getInstanceSettings();
    settings.msAnimClip = clip;
    settings.msCrossBlendDestAnimClip = clip;
    settings.msBlendingMode = blendMode::crossfade;
    settings.msAnimCrossBlendFactor = 1.0f;

    // ЖЕСТКО СБРАСЫВАЕМ В 0 и ОТКЛЮЧАЕМ АВТОМАТ:
    settings.msAnimTimePosition = 0.0f;
    settings.msPlayAnimation = false; // Теперь мы сами кормим время!
    settings.msAnimSpeed = 1.0f;

    enemy->getModel()->setInstanceSettings(settings);
    enemy->getModel()->checkForUpdates();

    // Тормозим физическое тело врага на время удара
    btRigidBody* body = enemy->getBody();
    if (body) {
        body->setLinearVelocity(btVector3(0, body->getLinearVelocity().y(), 0));
        body->setAngularVelocity(btVector3(0, 0, 0));
    }

    mAttackTimer = 0.0f;
    mAttackDuration = enemy->getModel()->getAnimationEndTime(clip);
    if (mAttackDuration <= 0.01f) {
        mAttackDuration = 1.0f; // Страховка
    }

    mHitWindows = enemy->getHitWindows(mActiveSlot);
    for (auto& w : mHitWindows) w.consumed = false;
}

void AIStateAttack::update(Enemy* enemy, float deltaTime) {
    // ВЫЗЫВАЕМ ЧЕРЕЗ enemy->
    if (enemy->shouldResetPosition()) {
        enemy->resetToSpawn();
        return;
    }
    mAttackTimer += deltaTime; // Мотаем локальный таймер

    auto settings = enemy->getModel()->getInstanceSettings();
    settings.msPlayAnimation = false;
    settings.msAnimTimePosition = mAttackTimer; // Насильно пихаем текущее время клипа
    enemy->getModel()->setInstanceSettings(settings);
    enemy->getModel()->checkForUpdates();


    float t = (mAttackDuration > 0.0f) ? (mAttackTimer / mAttackDuration) : 0.0f;

    // Проверяем все ещё не сработавшие окна урона (для Combo их может быть несколько)
    for (auto& window : mHitWindows) {
        if (window.consumed) continue;
        if (t >= window.startFrac && t <= window.endFrac) {
            btTransform trans;
            enemy->getBody()->getMotionState()->getWorldTransform(trans);
            glm::vec3 currentPos(trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z());

            glm::vec3 dirToTarget = enemy->getTargetPos() - currentPos;
            dirToTarget.y = 0.0f;
            float distance = glm::length(dirToTarget);

            // Небольшой запас сверх attackRange — чтобы удар не срывался, если игрок
            // чуть отошёл за то время, пока враг замахивался.
            float hitRange = enemy->getAttackRange() + 1.0f;

            if (distance <= hitRange) {
                enemy->setJustHitPlayer(true); // Сообщаем в движок!
                window.consumed = true;
                std::cout << "[ENEMY] Удар прошёл!" << std::endl;
            }
        }
    
    }
    enemy->onAttackPhysicsUpdate(mAttackTimer, deltaTime, mActiveSlot);
    // Если анимация удара полностью доиграла до конца
    if (mAttackTimer >= mAttackDuration) {
        // Возвращаемся в погоню за Энти
        enemy->changeState(std::make_unique<AIStateChase>());
    }
}

// ==========================================
// 4. ЛОГИКА ПОЛУЧЕНИЯ ХИТА (HIT REACTION)
// ==========================================
void AIStateHitReaction::enter(Enemy* enemy) {
    int clip = enemy->getAnimIndex(EnemyAnimSlot::HitReaction);
    if (clip < 0) {
        // У этого врага нет анимации реакции на урон (poise-боссы вроде Conway) —
        // сразу назад в погоню, ничего не проигрываем и не станим.
        enemy->changeState(std::make_unique<AIStateChase>());
        return;
    }
    // 2. ИНКРЕМЕНТИРУЕМ СЧЕТЧИК СТАНОВ
    enemy->incrementStunCount();

    auto settings = enemy->getModel()->getInstanceSettings();
    settings.msAnimClip = clip;
    settings.msCrossBlendDestAnimClip = clip;
    settings.msBlendingMode = blendMode::crossfade;
    settings.msAnimCrossBlendFactor = 1.0f;

    settings.msAnimTimePosition = 0.0f;
    settings.msPlayAnimation = false; // Кормим время руками!
    settings.msAnimSpeed = 1.0f;

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
        mStunDuration = 0.6f; // Страховка
    }
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

// ==========================================
// 5. ЛОГИКА СМЕРТИ (DEATH)
// ==========================================
void AIStateDeath::enter(Enemy* enemy) {
    int clip = enemy->getAnimIndex(EnemyAnimSlot::Death);

    auto settings = enemy->getModel()->getInstanceSettings();
    settings.msAnimClip = clip;
    settings.msCrossBlendDestAnimClip = clip;
    settings.msBlendingMode = blendMode::crossfade;
    settings.msAnimCrossBlendFactor = 1.0f;

    settings.msAnimTimePosition = 0.0f;
    settings.msPlayAnimation = false; // Будем крутить сами, чтобы остановить в конце!
    settings.msAnimSpeed = 1.0f;

    enemy->getModel()->setInstanceSettings(settings);
    enemy->getModel()->checkForUpdates();

    // Останавливаем физику и делаем труп "прозрачным" для игрока
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
    // Крутим анимацию, пока она не дойдет до последнего кадра
    if (mDeathTimer < mDeathDuration) {
        mDeathTimer += deltaTime;

        auto settings = enemy->getModel()->getInstanceSettings();
        settings.msAnimTimePosition = std::min(mDeathTimer, mDeathDuration - 0.01f);

        enemy->getModel()->setInstanceSettings(settings);
        enemy->getModel()->checkForUpdates();
    }
    // Когда анимация закончилась, больше ничего не делаем. Он просто лежит.
}

// ==========================================
// 6. ЛОГИКА БЛОКА (BLOCK) — новое
// ==========================================
void AIStateBlock::enter(Enemy* enemy) {
    int clip = enemy->getAnimIndex(EnemyAnimSlot::Block);
    if (clip < 0) {
        // Враг не умеет блокировать (нет клипа) — сразу в погоню.
        enemy->changeState(std::make_unique<AIStateChase>());
        return;
    }

    auto settings = enemy->getModel()->getInstanceSettings();
    settings.msAnimClip = clip;
    settings.msCrossBlendDestAnimClip = clip;
    settings.msBlendingMode = blendMode::crossfade;
    settings.msAnimCrossBlendFactor = 1.0f;
    settings.msAnimTimePosition = 0.0f;
    settings.msPlayAnimation = true; // блок можно просто зациклить автоматом
    settings.msAnimSpeed = 1.0f;

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