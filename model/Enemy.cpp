#include "Enemy.h"
#include "AIStates.h"

Enemy::Enemy(std::shared_ptr<GltfInstance> model, btRigidBody* body)
    : mModel(model), mBody(body) {
    // При спавне враг всегда появляется в состоянии "Ожидание"
    changeState(std::make_unique<AIStateIdle>());
}

Enemy::~Enemy() {}

void Enemy::update(float deltaTime, const glm::vec3& playerPos) {
    mTargetPos = playerPos; // Обновляем инфу о том, где Энти

    if (mCurrentState) {
        mCurrentState->update(this, deltaTime); // Мозг думает
    }
}

void Enemy::changeState(std::unique_ptr<AIState> newState) {
    mCurrentState = std::move(newState);
    if (mCurrentState) {
        mCurrentState->enter(this);
    }
}
bool Enemy::CanSeeTarget() const {
    if (!mBody) return false;

    // Получаем текущую позицию врага из Bullet Physics
    btTransform trans;
    mBody->getMotionState()->getWorldTransform(trans);
    glm::vec3 currentPos(trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z());

    // Вектор от врага к игроку (горизонтальный)
    glm::vec3 dirToTarget = mTargetPos - currentPos;
    dirToTarget.y = 0.0f; // Игнорируем разницу в высоте
    float distance = glm::length(dirToTarget);

    // ПРОВЕРКА 1: Радиус обнаружения
    if (distance > mDetectionRadius) {
        return false; // Игрок слишком далеко
    }

    if (distance > 0.1f) {
        dirToTarget = glm::normalize(dirToTarget);

        btVector3 btForward = quatRotate(trans.getRotation(), btVector3(0, 0, 1));
        glm::vec3 forward(btForward.x(), 0.0f, btForward.z());

        if (glm::length(forward) > 0.001f) {
            forward = glm::normalize(forward);
        }
        else {
            forward = glm::vec3(0.0f, 0.0f, 1.0f);
        }

        // Скалярное произведение (дает косинус угла между взглядом и игроком)
        float dotProduct = glm::dot(forward, dirToTarget);

        // Считаем минимально допустимый косинус на основе заданных градусов
        
        float targetCos = glm::cos(glm::radians(mFieldOfViewDegrees));

        // Чем МЕНЬШЕ скалярное произведение, тем БОЛЬШЕ угол.
        // Если dotProduct меньше targetCos — игрок вне зоны видимости.
        if (dotProduct < targetCos) {
            return false;
        }
    }

    return true; // Игрок близко и перед глазами!
}