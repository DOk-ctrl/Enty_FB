#define GLM_ENABLE_EXPERIMENTAL
#include "JigglePhysics.h"
#include <algorithm>
#include <cmath>

std::shared_ptr<GltfNode> findNodeByName(const std::shared_ptr<GltfNode>& node, const std::string& name) {
    if (!node) return nullptr;
    if (node->getNodeName() == name) return node;

    for (auto& child : node->getChilds()) {
        auto found = findNodeByName(child, name);
        if (found) return found;
    }
    return nullptr;
}

void initBreastJiggle(BreastJiggle& jiggle, std::shared_ptr<GltfNode> node, float stif, float damp, float Angle, float ex) {
    jiggle.node = node;
    jiggle.angularVelocity = glm::vec3(0.0f); // Ќ≈ (8,8,8) Ч это искусственный неубираемый толчок при спавне
    jiggle.jiggleOffset = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    jiggle.hasPrevPos = false;
    jiggle.prevVelocity = glm::vec3(0.0f);

    // Ѕезопасные стартовые значени€ Ч крутить отсюда, а не с экстремальных чисел
    jiggle.stiffness = stif;//40
    jiggle.damping = damp;//4
    jiggle.maxAngleDeg = Angle;//18
    jiggle.excitation = ex;//1.5
}

void updateBreastJiggle(BreastJiggle& jiggle, float deltaTime) {
    if (!jiggle.node) return;

    float dt = std::min(deltaTime, 1.0f / 30.0f);

    glm::quat animRot = jiggle.node->getLocalRotation();

    // ¬ј∆Ќќ: нужна ¬“ќ–јя производна€ позиции (ускорение), а не перва€ (скорость)!
    // (позици€ - позици€)/dt даЄт скорость Ч она у бегущего персонажа примерно
    // ѕќ—“ќяЌЌјя и Ќ≈Ќ”Ћ≈¬јя, а значит будет вливать энергию в пружину  ј∆ƒџ…
    // кадр без остановки -> разгон в бесконечность (именно это было в логе).
    // ”скорение реагирует только на »«ћ≈Ќ≈Ќ»≈ скорости (старт, торможение,
    // приземление) и само по себе спадает до нул€ на равномерном движении.
    glm::vec3 worldPos = jiggle.node->getGlobalPosition();

    glm::vec3 acceleration(0.0f);
    if (jiggle.hasPrevPos) {
        glm::vec3 velocity = (worldPos - jiggle.prevWorldPos) / dt;
        acceleration = (velocity - jiggle.prevVelocity) / dt;
        jiggle.prevVelocity = velocity;
    }
    else {
        jiggle.hasPrevPos = true;
    }
    jiggle.prevWorldPos = worldPos;

    glm::vec3 excitationTorque(
        -acceleration.z * jiggle.excitation + acceleration.y * jiggle.excitation,
        0.0f,
        acceleration.x * jiggle.excitation
    );
    jiggle.angularVelocity += excitationTorque * dt;

    // 2. ЅерЄм векторную часть текущего jiggleOffset как приближение
    //    "ось * угол" Ч корректно дл€ малых углов, а джигл-отклонени€
    //    по построению малые (максимум maxAngleDeg).
    glm::vec3 axisAngle(jiggle.jiggleOffset.x, jiggle.jiggleOffset.y, jiggle.jiggleOffset.z);
    if (jiggle.jiggleOffset.w < 0.0f) {
        //  ватернион и его негатив представл€ют одно и то же вращение Ч
        // берЄм "короткий путь", чтобы пружина не металась в другую сторону.
        axisAngle = -axisAngle;
    }

    // 3. ѕружина + затухание (простой демпфированный осцилл€тор на угловой скорости)
    glm::vec3 springTorque = -axisAngle * jiggle.stiffness;
    glm::vec3 dampingTorque = -jiggle.angularVelocity * jiggle.damping;
    jiggle.angularVelocity += (springTorque + dampingTorque) * dt;

    // 4. »нтегрируем угловую скорость в jiggleOffset (стандартна€ формула
    //    dq/dt = 0.5 * omega_quat * q)
    glm::quat velocityQuat(0.0f, jiggle.angularVelocity.x, jiggle.angularVelocity.y, jiggle.angularVelocity.z);
    jiggle.jiggleOffset = glm::normalize(jiggle.jiggleOffset + 0.5f * velocityQuat * jiggle.jiggleOffset * dt);

    // 5. ∆Єсткий клэмп угла отклонени€ Ч защита от срыва в дикое вращение
    float angleRad = 2.0f * std::acos(std::clamp(jiggle.jiggleOffset.w, -1.0f, 1.0f));
    float maxRad = glm::radians(jiggle.maxAngleDeg);
    if (angleRad > maxRad) {
        glm::vec3 axis(jiggle.jiggleOffset.x, jiggle.jiggleOffset.y, jiggle.jiggleOffset.z);
        float axisLen = glm::length(axis);
        if (axisLen > 0.0001f) {
            axis /= axisLen;
            jiggle.jiggleOffset = glm::angleAxis(maxRad, axis);
        }
        // √асим скорость при клэмпе, чтобы не "отталкивалось" от невидимой стенки бесконечно
        jiggle.angularVelocity *= 0.3f;
    }

    // 6. ‘инальна€ поза = анимационна€ поза + джигл-отклонение
    glm::quat finalRot = glm::normalize(animRot * jiggle.jiggleOffset);

    // ѕишем через blendRotation(final, 1.0f), а Ќ≈ setRotation() Ч
    // это обновл€ет mBlendRotation (то, что реально идЄт в mRotationMatrix
    // и, соответственно, в рендер), но Ќ≈ трогает mRotation Ч базу, от
    // которой blendAnimationFrame() будет считать slerp при следующем
    // кроссфейде между анимаци€ми (Idle -> Walk и т.п.). ≈сли писать через
    // setRotation(), кроссфейд начнЄт блендитьс€ от "испорченной" джиглом
    // позы, и это всплывЄт только в момент смены анимации.
    jiggle.node->blendRotation(finalRot, 1.0f);
}