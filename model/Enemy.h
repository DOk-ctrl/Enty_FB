#pragma once
#include <memory>
#include <glm/glm.hpp>
#include "btBulletDynamicsCommon.h"
#include "GltfInstance.h"

// ќпережающее объ€вление (чтобы не было циклических зависимостей)
class AIState;

class Enemy {
private:
    std::shared_ptr<GltfInstance> mModel; // —сылка на 3D модель врага
    btRigidBody* mBody;                   // —сылка на его физику
    std::unique_ptr<AIState> mCurrentState; // “екущее состо€ние (мозг)

    glm::vec3 mTargetPos;                 //  уда бежать (координаты Ёнти)
    float mSpeed = 4.0f;                  // —корость бега врага

    float mDetectionRadius = 32.0f;
    float mFieldOfViewDegrees = 60.0f; 
    // 120.0f Ч ¬раг видит отлично, у него очень широкое периферическое зрение. —боку к нему подойти т€жело.
    //90.0f Ч —тандартный, хороший сектор обзора.¬идит то, что перед ним.
    //  45.0f Ч ќчень узкий коридор зрени€.—имбиот будет замечать теб€, только если ты стоишь пр€мо перед его носом.—боку и сзади он абсолютно слеп.

public:
    Enemy(std::shared_ptr<GltfInstance> model, btRigidBody* body);
    ~Enemy();

    // √лавный метод, который мы будем дергать каждый кадр из движка
    void update(float deltaTime, const glm::vec3& playerPos);

    // ѕереключатель состо€ний
    void changeState(std::unique_ptr<AIState> newState);

    bool CanSeeTarget() const;
    // √еттеры дл€ доступа из состо€ний
    btRigidBody* getBody() { return mBody; }
    std::shared_ptr<GltfInstance> getModel() { return mModel; }
    glm::vec3 getTargetPos() const { return mTargetPos; }
    float getSpeed() const { return mSpeed; }
};