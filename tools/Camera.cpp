#define GLM_ENABLE_EXPERIMENTAL
#define _USE_MATH_DEFINES
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"

glm::mat4 Camera::getViewMatrix(OGLRenderData &renderData) {
  float azimRad = glm::radians(renderData.rdViewAzimuth);
  float elevRad = glm::radians(renderData.rdViewElevation);

  float sinAzim = glm::sin(azimRad);
  float cosAzim = glm::cos(azimRad);
  float sinElev = glm::sin(elevRad);
  float cosElev = glm::cos(elevRad);

  /* update view direction */
  mViewDirection = glm::normalize(glm::vec3(
     sinAzim * cosElev, sinElev, -cosAzim * cosElev));

  /* calculate right and up direction */
  mRightDirection = glm::normalize(glm::cross(mViewDirection, mWorldUpVector));
  mUpDirection = glm::normalize(glm::cross(mRightDirection, mViewDirection));

  /* update camera position depending on desired movement */
  renderData.rdCameraWorldPosition +=
    renderData.rdMoveForward * renderData.rdTickDiff * mViewDirection
    + renderData.rdMoveRight * renderData.rdTickDiff * mRightDirection
    + renderData.rdMoveUp * renderData.rdTickDiff * mUpDirection;

  return glm::lookAt(renderData.rdCameraWorldPosition,
    renderData.rdCameraWorldPosition + mViewDirection, mUpDirection);
}

glm::mat4 Camera::getThirdPersonViewMatrix(OGLRenderData& renderData, glm::vec3 targetPosition) {
    float azimRad = glm::radians(renderData.rdViewAzimuth);
    float elevRad = glm::radians(renderData.rdViewElevation);

    float sinAzim = glm::sin(azimRad);
    float cosAzim = glm::cos(azimRad);
    float sinElev = glm::sin(elevRad);
    float cosElev = glm::cos(elevRad);

    mViewDirection = glm::normalize(glm::vec3(sinAzim * cosElev, sinElev, -cosAzim * cosElev));
    mRightDirection = glm::normalize(glm::cross(mViewDirection, mWorldUpVector));
    mUpDirection = glm::normalize(glm::cross(mRightDirection, mViewDirection));

    // 1. Точка фокуса
    glm::vec3 headPos = targetPosition + glm::vec3(0.0f, 1.4f, 0.0f);

    // 2. ПЛАВНЫЙ QUICK ZOOM (Исправление дерганости)
    static float currentCamDistance = 2.5f; // Хранит текущую дистанцию между кадрами
    float targetDistance = 2.5f;            // Обычная дистанция

    if (renderData.rdZoomTimer > 0.0f) {
        targetDistance = 1.2f;              // Целевая дистанция при ударе
        renderData.rdZoomTimer -= renderData.rdTickDiff;
    }

    // Скорость наезда/отхода камеры (20.0f дает очень быстрый, но мягкий наезд)
    float zoomSpeed = 20.0f * renderData.rdTickDiff;
    if (zoomSpeed > 1.0f) zoomSpeed = 1.0f;

    // Интерполируем текущую дистанцию к целевой
    currentCamDistance = glm::mix(currentCamDistance, targetDistance, zoomSpeed);

    // 3. Вычисляем позицию с плавно изменяющейся дистанцией
    glm::vec3 offset = (mRightDirection * 0.6f) + (mUpDirection * 0.2f) - (mViewDirection * currentCamDistance);
    glm::vec3 desiredPosition = headPos + offset;

    // 4. Плавное следование всей камеры за игроком (Lerp)
    float smoothSpeed = 5.0f * renderData.rdTickDiff;
    if (smoothSpeed > 1.0f) smoothSpeed = 1.0f;

    renderData.rdCameraWorldPosition = glm::mix(renderData.rdCameraWorldPosition, desiredPosition, smoothSpeed);

    // 5. SCREEN SHAKE (Тряска камеры)
    glm::vec3 shakeOffset(0.0f);
    if (renderData.rdShakeDuration > 0.0f) {
        renderData.rdShakeDuration -= renderData.rdTickDiff;

        float shakeX = ((rand() % 100) / 100.0f - 0.5f) * 2.0f * renderData.rdShakeIntensity;
        float shakeY = ((rand() % 100) / 100.0f - 0.5f) * 2.0f * renderData.rdShakeIntensity;
        float shakeZ = ((rand() % 100) / 100.0f - 0.5f) * 2.0f * renderData.rdShakeIntensity;

        shakeOffset = (mRightDirection * shakeX) + (mUpDirection * shakeY) + (mViewDirection * shakeZ);
    }

    glm::vec3 finalCamPos = renderData.rdCameraWorldPosition + shakeOffset;
    glm::vec3 lookAtTarget = finalCamPos + mViewDirection;

    return glm::lookAt(finalCamPos, lookAtTarget, mUpDirection);
}