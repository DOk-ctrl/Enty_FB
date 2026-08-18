#pragma once

#include <vector>
#include <memory>

// Форвард-декларации для типов из GLFW и твоих классов
struct GLFWwindow; // <-- Вместо тяжелого инклуда пишем это!
class GltfModel;
struct OGLRenderData;

/*
 * GraphicsManager
 * ----------------
 * Не хранит собственных "настроек" - источник истины это поля
 * rdVSyncIndex / rdFullscreenIndex / rdResolutionIndex / rdAnisotropyIndex
 * в OGLRenderData, которые пишет UI (см. UserInterface.cpp).
 *
 * update() вызывается один раз в кадр (см. OGLRenderer::draw(), рядом
 * с уже существующей проверкой rdTextureQuality) и применяет изменение
 * только когда значение реально отличается от последнего применённого -
 * тот же паттерн, что уже используется в проекте для качества текстур.
 *
 * MSAA сюда не входит: смена числа samples требует пересоздания
 * MSFramebuffer конкретного размера, поэтому эта часть обрабатывается
 * прямо в OGLRenderer::draw() (там же, где выбирается, в какой FBO
 * рисовать сцену).
 */
class GraphicsManager {
public:
    // Вызывать один раз в кадр
    void update(OGLRenderData& rd, std::vector<std::shared_ptr<GltfModel>>& models);

private:
    void applyVSync(OGLRenderData& rd);
    void applyWindowMode(OGLRenderData& rd);
    void applyAnisotropy(OGLRenderData& rd, std::vector<std::shared_ptr<GltfModel>>& models);
    void applyLanguage(OGLRenderData& rd);
    static void resolutionFromIndex(int index, int& width, int& height);
    static float anisotropyFromIndex(int index);

    // Кэш последних применённых значений (аналог lastTextureQuality)
    int mLastVSync = -1;
    int mLastFullscreen = -1;
    int mLastResolution = -1;
    int mLastAnisotropy = -1;
    int mLastLanguage = -1;

    // Геометрия окна перед уходом в fullscreen - чтобы вернуться в то же место
    int mWindowedPosX = 100;
    int mWindowedPosY = 100;
    int mWindowedWidth = 1280;
    int mWindowedHeight = 720;
};
