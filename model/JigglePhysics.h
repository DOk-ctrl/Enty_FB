#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "GltfNode.h"

// ============================================================
// ƒжигл-кость (вторична€ анимаци€) дл€ одной кости груди.
//
// »де€: анимаци€ каждый кадр кладЄт в mBlendRotation "чистую" позу этой кости
// (через setAnimationFrame -> setRotation() или blendAnimationFrame ->
// blendRotation()). ћы читаем эту чистую позу через getLocalRotation(),
// добавл€ем к ней небольшое пружинное отклонение (jiggleOffset) и пишем
// результат назад через blendRotation(final, 1.0f) Ч это перезаписывает
// –ќ¬Ќќ то же значение, что дал бы обычный setRotation(final), но не трогает
// mRotation, от которой в будущем считаетс€ blend при смене клипа
// (Idle -> Walk и т.п.). »менно тут была главна€ ловушка в исходном коде.
// ============================================================
struct BreastJiggle {
    std::shared_ptr<GltfNode> node;

    glm::vec3 angularVelocity{ 0.0f };                       // "углова€ скорость" довеска, рад/с
    glm::quat jiggleOffset{ 1.0f, 0.0f, 0.0f, 0.0f };        // текущее отклонение от анимационной позы

    // --- ¬озбуждение от реального движени€ тела (то, чего не было и почему джигл был незаметен) ---
    glm::vec3 prevWorldPos{ 0.0f };
    glm::vec3 prevVelocity{ 0.0f };
    bool hasPrevPos = false;

    float stiffness = 120.0f; // сила, т€нуща€ обратно к анимационной позе Ч чем больше, тем "жЄстче" грудь
    float damping = 6.0f;   // затухание колебаний Ч чем больше, тем быстрее гаснет дрожание
    float maxAngleDeg = 12.0f;  // жЄсткий предел отклонени€ в градусах, чтобы не "срывало" при резких движени€х
    float excitation = 0.35f;  // √Ћј¬Ќјя ручка "силы тр€ски" Ч насколько ускорение тела раскачивает пружину
};

// »щет узел по имени в дереве скелета (рекурсивно). ¬ызови один раз при инициализации,
// начина€ с mGltfInstances.at(N)->getRootNode(). ≈сли имена костей после импорта в
// движок отличаютс€ от того, что ты назвал в Blender (Mixamo иногда добавл€ет префиксы,
// см. "mixamorig:Hips" в GltfAnimationClip.cpp) Ч сначала вызови
// rootNode->printTree() и посмотри реальные имена в логе.
std::shared_ptr<GltfNode> findNodeByName(const std::shared_ptr<GltfNode>& node, const std::string& name);

// »нициализаци€ Ч один раз после загрузки модели/скелета.
void initBreastJiggle(BreastJiggle& jiggle, std::shared_ptr<GltfNode> node, float stif, float damp, float Angle, float ex);

// ¬ызывать каждый кадр ѕќ—Ћ≈ instance->updateAnimation() (и после solveIK(), если он
// тоже трогает эти кости Ч вр€д ли, но лучше после). —разу за парой вызовов
// updateBreastJiggle() дл€ левой/правой кости нужно один раз вызвать
// rootNode->updateNodeAndChildMatrices(), чтобы пересчитать мировые матрицы
// с учЄтом джигла (см. пример подключени€ в OGLRenderer).
void updateBreastJiggle(BreastJiggle& jiggle, float deltaTime);