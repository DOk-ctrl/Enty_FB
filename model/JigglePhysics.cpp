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
    jiggle.angularVelocity = glm::vec3(0.0f);     jiggle.jiggleOffset = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    jiggle.hasPrevPos = false;
    jiggle.prevVelocity = glm::vec3(0.0f);

        jiggle.stiffness = stif;    jiggle.damping = damp;    jiggle.maxAngleDeg = Angle;    jiggle.excitation = ex;}

void updateBreastJiggle(BreastJiggle& jiggle, float deltaTime) {
    if (!jiggle.node) return;

    float dt = std::min(deltaTime, 1.0f / 30.0f);

    glm::quat animRot = jiggle.node->getLocalRotation();

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

                glm::vec3 axisAngle(jiggle.jiggleOffset.x, jiggle.jiggleOffset.y, jiggle.jiggleOffset.z);
    if (jiggle.jiggleOffset.w < 0.0f) {
                        axisAngle = -axisAngle;
    }

        glm::vec3 springTorque = -axisAngle * jiggle.stiffness;
    glm::vec3 dampingTorque = -jiggle.angularVelocity * jiggle.damping;
    jiggle.angularVelocity += (springTorque + dampingTorque) * dt;

            glm::quat velocityQuat(0.0f, jiggle.angularVelocity.x, jiggle.angularVelocity.y, jiggle.angularVelocity.z);
    jiggle.jiggleOffset = glm::normalize(jiggle.jiggleOffset + 0.5f * velocityQuat * jiggle.jiggleOffset * dt);

        float angleRad = 2.0f * std::acos(std::clamp(jiggle.jiggleOffset.w, -1.0f, 1.0f));
    float maxRad = glm::radians(jiggle.maxAngleDeg);
    if (angleRad > maxRad) {
        glm::vec3 axis(jiggle.jiggleOffset.x, jiggle.jiggleOffset.y, jiggle.jiggleOffset.z);
        float axisLen = glm::length(axis);
        if (axisLen > 0.0001f) {
            axis /= axisLen;
            jiggle.jiggleOffset = glm::angleAxis(maxRad, axis);
        }
                jiggle.angularVelocity *= 0.3f;
    }

        glm::quat finalRot = glm::normalize(animRot * jiggle.jiggleOffset);

                                jiggle.node->blendRotation(finalRot, 1.0f);
}