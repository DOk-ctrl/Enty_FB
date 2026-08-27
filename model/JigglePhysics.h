#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "GltfNode.h"

struct BreastJiggle {
    std::shared_ptr<GltfNode> node;

    glm::vec3 angularVelocity{ 0.0f };                           glm::quat jiggleOffset{ 1.0f, 0.0f, 0.0f, 0.0f };        
        glm::vec3 prevWorldPos{ 0.0f };
    glm::vec3 prevVelocity{ 0.0f };
    bool hasPrevPos = false;

    float stiffness = 120.0f;     float damping = 6.0f;       float maxAngleDeg = 12.0f;      float excitation = 0.35f;  };

std::shared_ptr<GltfNode> findNodeByName(const std::shared_ptr<GltfNode>& node, const std::string& name);

void initBreastJiggle(BreastJiggle& jiggle, std::shared_ptr<GltfNode> node, float stif, float damp, float Angle, float ex);

void updateBreastJiggle(BreastJiggle& jiggle, float deltaTime);