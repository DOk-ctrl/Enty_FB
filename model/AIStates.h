#pragma once
#include <glm/glm.hpp>
#include <GltfInstance.h>

class Enemy;

// Базовый класс состояния
class AIState {
public:
    virtual ~AIState() = default;
    virtual void enter(Enemy* enemy) = 0;
    virtual void update(Enemy* enemy, float deltaTime) = 0;
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

// 3. Состояние: АТАКУЕТ
class AIStateAttack : public AIState {
private:
    float mAttackTimer = 0.0f;
public:
    void enter(Enemy* enemy) override;
    void update(Enemy* enemy, float deltaTime) override;
};