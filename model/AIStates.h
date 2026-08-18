#pragma once
#include <glm/glm.hpp>
#include <GltfInstance.h>
#include "EnemyTypes.h"

class Enemy;

// Базовый класс состояния
class AIState {
public:
    virtual ~AIState() = default;
    virtual void enter(Enemy* enemy) = 0;
    virtual void update(Enemy* enemy, float deltaTime) = 0;
    virtual void exit(Enemy* /*enemy*/) {} // по умолчанию ничего не делаем
};

// 1. Состояние: СТОИТ И ЖДЕТ
class AIStateIdle : public AIState {
private:
    float mWaitTimer = 0.0f;
public:
    void enter(Enemy* enemy) override;
    void update(Enemy* enemy, float deltaTime) override;
};

// 2. Состояние: БЕЖИТ ЗА ЭНТИ
class AIStateChase : public AIState {
public:
    void enter(Enemy* enemy) override;
    void update(Enemy* enemy, float deltaTime) override;
};

// 3. Состояние: АТАКУЕТ — теперь генерик: сам выбирает слот атаки у enemy
// и поддерживает несколько окон урона за одну анимацию (Combo).
class AIStateAttack : public AIState {
private:
    float mAttackTimer = 0.0f;
    float mAttackDuration = 0.0f;
    EnemyAnimSlot mActiveSlot = EnemyAnimSlot::Attack1;
    std::vector<HitWindow> mHitWindows;
public:
    void enter(Enemy* enemy) override;
    void update(Enemy* enemy, float deltaTime) override;
};

// состояние получает урон
class AIStateHitReaction : public AIState {
private:
    float mStunTimer = 0.0f;
    float mStunDuration = 0.0f;
public:
    void enter(Enemy* enemy) override;
    void update(Enemy* enemy, float deltaTime) override;
};

// 5. Состояние: СМЕРТЬ
class AIStateDeath : public AIState {
private:
    float mDeathDuration = 0.0f;
    float mDeathTimer = 0.0f;
public:
    void enter(Enemy* enemy) override;
    void update(Enemy* enemy, float deltaTime) override;
};

// 6. НОВОЕ Состояние: БЛОК — доступно только врагам с canBlock() == true.
// Если у врага нет клипа Block (getAnimIndex возвращает -1), enter() сразу
// откатывается в Chase, так что этот стейт безопасно ставить кому угодно.
class AIStateBlock : public AIState {
private:
    float mBlockTimer = 0.0f;
    float mBlockDuration = 1.2f;
public:
    void enter(Enemy* enemy) override;
    void update(Enemy* enemy, float deltaTime) override;
    void exit(Enemy* enemy) override;
};