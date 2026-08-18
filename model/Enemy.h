#pragma once
#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "btBulletDynamicsCommon.h"
#include "GltfInstance.h"
#include "EnemyTypes.h"

// Опережающее объявление (чтобы не было циклических зависимостей)
class AIState;

// Базовый враг. Раньше был "только симбиот" — теперь это база для всех
// врагов (симбиот + боссы), а конкретные наборы анимаций/атак задаются
// через виртуальные методы в производных классах (см. BossConway, BossKicker).
class Enemy {
protected:
    int mStunCount = 0; // Счетчик получений стана
    std::shared_ptr<GltfInstance> mModel; // Ссылка на 3D модель врага
    btRigidBody* mBody;                   // Ссылка на его физику
    std::unique_ptr<AIState> mCurrentState; // Текущее состояние (мозг)

    glm::vec3 mTargetPos{};               // Куда бежать (координаты Энти)
    float mSpeed;                         // Скорость бега врага
    float mDetectionRadius;
    float mFieldOfViewDegrees;
    // 120.0f — очень широкое периферическое зрение.
    // 90.0f — стандартный сектор.
    // 45.0f — узкий коридор, слеп сбоку и сзади.
    float mHealth;
    float mMaxHealth;
    bool  mIsBlocking = false;            // true пока враг в состоянии Block
    bool  mHasEngagedPlayer = false;      // true с первого раза, как увидел игрока (для boss health bar)
    glm::vec3 mSpawnPos = glm::vec3(0.0f);
public:
    Enemy(std::shared_ptr<GltfInstance> model, btRigidBody* body,
        float maxHealth = 100.0f, float speed = 4.0f,
        float detectionRadius = 32.0f, float fovDegrees = 70.0f);

    // ВАЖНО: virtual — Enemy хранится полиморфно в std::vector<unique_ptr<Enemy>>,
    // без virtual деструктора удаление через unique_ptr<Enemy> было бы UB.
    virtual ~Enemy();

    bool isDead() const { return mHealth <= 0.0f; }
    bool isBlocking() const { return mIsBlocking; }
    void setBlocking(bool blocking) { mIsBlocking = blocking; }

    int getStunCount() const { return mStunCount; }
    void incrementStunCount() { mStunCount++; }
    // Раз войдя в бой (увидел игрока -> Chase), остаётся "вовлечённым" до смерти —
    // именно на этом флаге держится показ boss health bar (не мигает при потере FOV).
    bool isEngaged() const { return mHasEngagedPlayer; }
    void setEngaged(bool engaged) { mHasEngagedPlayer = engaged; }

    // Возвращает true, если враг нарушил границы арены/зоны
    virtual bool shouldResetPosition() const { return false; }
    virtual glm::vec3 getSpawnPosition() const { return mSpawnPos; } // Или твоя начальная точка
    void resetToSpawn();
    // Показывать ли для этого врага полосу здоровья босса (DS3/Elden Ring style).
    virtual bool isBoss() const { return false; }
    virtual std::string getBossName() const { return "ENEMY"; }

    // Главный метод, который дергается каждый кадр из движка
    virtual void update(float deltaTime, const glm::vec3& playerPos);
    // Вызывается из AIStateAttack каждый кадр, пока идет анимация атаки
    virtual void onAttackPhysicsUpdate(float attackTimeAccumulator, float deltaTime, EnemyAnimSlot currentAttackAnim) {}
    // Переключатель состояний
    void changeState(std::unique_ptr<AIState> newState);

    bool mJustHitPlayer = false;
    void setJustHitPlayer(bool hit);
    bool consumeHitPlayerFlag();
    bool CanSeeTarget() const;

    // Геттеры для доступа из состояний
    btRigidBody* getBody() { return mBody; }
    virtual void TakeDamage(float amount);
    std::shared_ptr<GltfInstance> getModel() { return mModel; }
    glm::vec3 getTargetPos() const { return mTargetPos; }
    float getSpeed() const { return mSpeed; }
    float getHealth() const { return mHealth; }
    float getMaxHealth() const { return mMaxHealth; }

    // ================================================================
    // ТОЧКИ РАСШИРЕНИЯ ДЛЯ БОССОВ / НОВЫХ ТИПОВ ВРАГОВ
    // ================================================================

    // Индекс клипа анимации для данного "слота". Каждый враг сам знает
    // порядок своих клипов (см. лог GltfModel::getAnimations при загрузке).
    // -1 = "этот враг так не умеет" (напр. Block у Symbiote).
    virtual int getAnimIndex(EnemyAnimSlot slot) const;

    // Какие атаки вообще есть у этого врага — используется для случайного
    // выбора атаки в AIStateAttack. По умолчанию — только Attack1 (симбиот).
    virtual std::vector<EnemyAnimSlot> getAvailableAttackSlots() const;
    virtual EnemyAnimSlot chooseAttackSlot() const;

    // Окна урона внутри анимации атаки (может быть несколько — Combo).
    virtual std::vector<HitWindow> getHitWindows(EnemyAnimSlot attackSlot) const;

    virtual float getAttackRange() const { return 1.5f; }
    virtual bool  canBlock() const { return false; }
    virtual bool  canBeStaggered() const { return true; } // false = "poise", боссы игнорируют стан
    virtual float getBlockDamageMultiplier() const { return 0.15f; } // % урона, проходящего сквозь блок
};