/* OpenGL */
#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <vector>
#include <string>

#include <glm/glm.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "GameStateMachine.h"

struct OGLVertex {
  glm::vec3 position;
  glm::vec3 color;
  glm::vec2 uv;
};

struct OGLMesh {
  std::vector<OGLVertex> vertices;
};

enum class skinningMode {
  linear = 0,
  dualQuat
};

enum class replayDirection {
  forward = 0,
  backward
};

enum class blendMode {
  fadeinout = 0,
  crossfade,
  additive
};

enum class ikMode {
  off = 0,
  ccd,
  fabrik
};

struct OGLRenderData {
  GLFWwindow *rdWindow = nullptr;

  int rdWidth = 0;
  int rdHeight = 0;

  unsigned int rdTriangleCount = 0;
  unsigned int rdGltfTriangleCount = 0;

  int rdFieldOfView = 60;

  float rdFrameTime = 0.0f;
  float rdMatrixGenerateTime = 0.0f;
  float rdIKTime = 0.0f;
  float rdUploadToVBOTime = 0.0f;
  float rdUploadToUBOTime = 0.0f;
  float rdUIGenerateTime = 0.0f;
  float rdUIDrawTime = 0.0f;

  int rdMoveForward = 0;
  int rdMoveRight = 0;
  int rdMoveUp = 0;

  float rdTickDiff = 0.0f;

  float rdViewAzimuth = 15.0f;
  float rdViewElevation = -25.0f;
  glm::vec3 rdCameraWorldPosition = glm::vec3(-10.0f, 16.0f, 35.0f);

  int rdNumberOfInstances = 0;
  int rdCurrentSelectedInstance = 0;
  bool mIsDeveloperMode = false;
  bool showCreditsWindow = false;
  bool rdMenuAnyButtonHovered = false;  // ImGui hover над кнопками меню

  int rdTextureQuality = 0; // 0 = Ультра, 1 = Высокие, 2 = Средние, 3 = Низкие
  int rdResolutionIndex = 1;   // 0:1280x720 1:1920x1080 2:2560x1440 3:3840x2160
  int rdFullscreenIndex = 0;   // 0 - Оконный, 1 - Fullscreen
  int rdVSyncIndex = 1;   // 0 - Выкл, 1 - Вкл
  int rdMSAAIndex = 3;   // 0..3 -> Off, 2x, 4x, 8x
  int rdAnisotropyIndex = 4;   // 0..4 -> Off, 2x, 4x, 8x, 16x
  int rdLanguageIndex = 0;
  // =========================================================================
  // НАСТРОЙКИ ОСВЕЩЕНИЯ ДЛЯ ШЕЙДЕРОВ (СЛАЙДЕРЫ)
  // =========================================================================

  //Sun
  glm::vec3 rdSunDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
  glm::vec3 rdSunColor = glm::vec3(1.0f, 1.0f, 0.9f);
  float rdSunIntensity = 1.0f;
  // Point Light
  glm::vec3 rdLightPos = glm::vec3(2.0f, 4.0f, 2.0f);
  glm::vec3 rdPointColor = glm::vec3(1.0f, 0.5f, 0.0f);
  float rdPointIntensity = 2.0f;
  float rdPointAttenuation = 0.5f; // Коэффициент затухания (чем выше, тем ближе гаснет свет)

  float rdPlayerHP = 100.0f;
  float rdPlayerMaxHP = 100.0f;
  bool rdPlayerIsDead = false;

  bool ShowDebugMenu;
  // --- ЭФФЕКТЫ БОЯ И КАМЕРЫ ---
  float rdHitStopTimer = 0.0f;     // Таймер заморозки времени (Hit Stop)

  float rdShakeDuration = 0.0f;    // Длительность тряски
  float rdShakeIntensity = 0.0f;   // Интенсивность тряски

  float rdZoomTimer = 0.0f;        // Таймер зума при фатальном ударе
  float rdTargetFOV = 60.0f;       // FOV во время зума (стандартный, например, 45.0f)
};
