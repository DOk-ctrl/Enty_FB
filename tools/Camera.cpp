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

    // Вектор направления взгляда камеры
    mViewDirection = glm::normalize(glm::vec3(sinAzim * cosElev, sinElev, -cosAzim * cosElev));
    mRightDirection = glm::normalize(glm::cross(mViewDirection, mWorldUpVector));
    mUpDirection = glm::normalize(glm::cross(mRightDirection, mViewDirection));

    // 1. Точка фокуса (целимся в район шеи/головы персонажа)
    glm::vec3 headPos = targetPosition + glm::vec3(0.0f, 1.4f, 0.0f);

    // 2. Смещение камеры (Offset). 
    // Вправо на 0.6м, вверх на 0.2м, назад на 2.5м. (Как в RE2)
    glm::vec3 offset = (mRightDirection * 0.6f) + (mUpDirection * 0.2f) - (mViewDirection * 2.5f);

    glm::vec3 desiredPosition = headPos + offset;

    // 3. Плавное следование камеры (Lerp)
    float smoothSpeed = 15.0f * renderData.rdTickDiff;
    if (smoothSpeed > 1.0f) smoothSpeed = 1.0f; // Защита от рывков при лагах

    renderData.rdCameraWorldPosition = glm::mix(renderData.rdCameraWorldPosition, desiredPosition, smoothSpeed);

    // Камера смотрит немного вперед от персонажа
    glm::vec3 lookAtTarget = renderData.rdCameraWorldPosition + mViewDirection;

    return glm::lookAt(renderData.rdCameraWorldPosition, lookAtTarget, mUpDirection);
}