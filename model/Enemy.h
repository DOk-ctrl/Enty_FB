#pragma once
#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "btBulletDynamicsCommon.h"
#include "GltfInstance.h"
#include "EnemyTypes.h"

class AIState;

class Enemy {
protected:
    int mStunCount = 0;    
    std::shared_ptr<GltfInstance> mModel;   
    btRigidBody* mBody;                    
    std::unique_ptr<AIState> mCurrentState; 
    glm::vec3 mTargetPos{};                
    float mSpeed;                         
    float mDetectionRadius;
    float mFieldOfViewDegrees;
                float mHealth;
    float mMaxHealth;
    bool  mIsBlocking = false;              
    bool  mHasEngagedPlayer = false;    
    glm::vec3 mSpawnPos = glm::vec3(0.0f);
public:
    Enemy(std::shared_ptr<GltfInstance> model, btRigidBody* body,
        float maxHealth = 100.0f, float speed = 4.0f,
        float detectionRadius = 32.0f, float fovDegrees = 160.0f);

 
        virtual ~Enemy();

    bool isDead() const { return mHealth <= 0.0f; }
    bool isBlocking() const { return mIsBlocking; }
    void setBlocking(bool blocking) { mIsBlocking = blocking; }
    virtual void playmusic();
    int getStunCount() const { return mStunCount; }
    void incrementStunCount() { mStunCount++; }
            bool isEngaged() const { return mHasEngagedPlayer; }
    void setEngaged(bool engaged) { mHasEngagedPlayer = engaged; }

    virtual bool isBismark() const { return false; }

        virtual bool shouldResetPosition() const { return false; }
    virtual glm::vec3 getSpawnPosition() const { return mSpawnPos; }     void resetToSpawn();
 
    virtual bool isBoss() const { return false; }
    virtual std::string getBossName() const { return "ENEMY"; }

        virtual void update(float deltaTime, const glm::vec3& playerPos);
        virtual void onAttackPhysicsUpdate(float attackTimeAccumulator, float deltaTime, EnemyAnimSlot currentAttackAnim) {}
        void changeState(std::unique_ptr<AIState> newState);

    bool mJustHitPlayer = false;
    void setJustHitPlayer(bool hit);
    bool consumeHitPlayerFlag();
    bool CanSeeTarget() const;

        btRigidBody* getBody() { return mBody; }
    virtual void TakeDamage(float amount);
    std::shared_ptr<GltfInstance> getModel() { return mModel; }
    glm::vec3 getTargetPos() const { return mTargetPos; }
    float getSpeed() const { return mSpeed; }
    float getHealth() const { return mHealth; }
    float getMaxHealth() const { return mMaxHealth; }

    
        virtual int getAnimIndex(EnemyAnimSlot slot) const;

            virtual std::vector<EnemyAnimSlot> getAvailableAttackSlots() const;
    virtual EnemyAnimSlot chooseAttackSlot() const;

        virtual std::vector<HitWindow> getHitWindows(EnemyAnimSlot attackSlot) const;

    virtual float getAttackRange() const { return 1.5f; }
    virtual bool  canBlock() const { return false; }
    virtual bool  canBeStaggered() const { return true; } 
    virtual float getBlockDamageMultiplier() const { return 0.15f; } 
};