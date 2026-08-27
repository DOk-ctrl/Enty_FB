#pragma once
#include <glm/glm.hpp>
#include <GltfInstance.h>
#include "EnemyTypes.h"

class Enemy;

class AIState {
public:
    virtual ~AIState() = default;
    virtual void enter(Enemy* enemy) = 0;
    virtual void update(Enemy* enemy, float deltaTime) = 0;
    virtual void exit(Enemy* /*enemy*/) {} };

class AIStateIdle : public AIState {
private:
    float mWaitTimer = 0.0f;
public:
    void enter(Enemy* enemy) override;
    void update(Enemy* enemy, float deltaTime) override;
};

class AIStateChase : public AIState {
public:
    void enter(Enemy* enemy) override;
    void update(Enemy* enemy, float deltaTime) override;
};

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

class AIStateHitReaction : public AIState {
private:
    float mStunTimer = 0.0f;
    float mStunDuration = 0.0f;
public:
    void enter(Enemy* enemy) override;
    void update(Enemy* enemy, float deltaTime) override;
};

class AIStateDeath : public AIState {
private:
    float mDeathDuration = 0.0f;
    float mDeathTimer = 0.0f;
public:
    void enter(Enemy* enemy) override;
    void update(Enemy* enemy, float deltaTime) override;
};

class AIStateBlock : public AIState {
private:
    float mBlockTimer = 0.0f;
    float mBlockDuration = 1.2f;
public:
    void enter(Enemy* enemy) override;
    void update(Enemy* enemy, float deltaTime) override;
    void exit(Enemy* enemy) override;
};