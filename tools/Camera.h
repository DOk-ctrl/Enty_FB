/* Simple camera object */
#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include "OGLRenderData.h"

class Camera {
  public:
    glm::mat4 getViewMatrix(OGLRenderData &renderData);

    glm::mat4 getThirdPersonViewMatrix(OGLRenderData& renderData, glm::vec3 targetPosition);

    
  private:
      glm::vec3 mViewDirection = glm::vec3(0.0f, 0.0f, 0.0f);
      glm::vec3 mRightDirection = glm::vec3(0.0f, 0.0f, 0.0f);
      glm::vec3 mUpDirection = glm::vec3(0.0f, 0.0f, 0.0f);

      /* world up is positive Y */
      glm::vec3 mWorldUpVector = glm::vec3(0.0f, 1.0f, 0.0f);
      
};
