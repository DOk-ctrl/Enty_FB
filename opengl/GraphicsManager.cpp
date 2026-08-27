#define GLM_ENABLE_EXPERIMENTAL
#include <algorithm>

#define GLFW_INCLUDE_NONE  #include <GLFW/glfw3.h>

#include "GraphicsManager.h"
#include "OGLRenderData.h"
#include "GltfModel.h"
#include "Logger.h"
#include "Localization.h"

void GraphicsManager::resolutionFromIndex(int index, int& width, int& height) {
    static const int table[4][2] = {
        {1280, 720}, {1920, 1080}, {2560, 1440}, {3840, 2160}
    };
    int i = std::clamp(index, 0, 3);
    width = table[i][0];
    height = table[i][1];
}

float GraphicsManager::anisotropyFromIndex(int index) {
    static const float table[5] = { 1.0f, 2.0f, 4.0f, 8.0f, 16.0f };
    int i = std::clamp(index, 0, 4);
    return table[i];
}

void GraphicsManager::update(OGLRenderData& rd, std::vector<std::shared_ptr<GltfModel>>& models) {
    if (rd.rdVSyncIndex != mLastVSync) {
        mLastVSync = rd.rdVSyncIndex;
        applyVSync(rd);
    }

    if (rd.rdFullscreenIndex != mLastFullscreen || rd.rdResolutionIndex != mLastResolution) {
        mLastFullscreen = rd.rdFullscreenIndex;
        mLastResolution = rd.rdResolutionIndex;
        applyWindowMode(rd);
    }

    if (rd.rdAnisotropyIndex != mLastAnisotropy) {
        mLastAnisotropy = rd.rdAnisotropyIndex;
        applyAnisotropy(rd, models);
    }
    if (rd.rdLanguageIndex != mLastLanguage) {
        mLastLanguage = rd.rdLanguageIndex;
        applyLanguage(rd);
    }
}

void GraphicsManager::applyVSync(OGLRenderData& rd) {
        glfwSwapInterval(rd.rdVSyncIndex);
    Logger::log(1, "%s: VSync set to %d\n", __FUNCTION__, rd.rdVSyncIndex);
}

void GraphicsManager::applyWindowMode(OGLRenderData& rd) {
    int width = 0, height = 0;
    resolutionFromIndex(rd.rdResolutionIndex, width, height);

    if (rd.rdFullscreenIndex == 1) {
                                if (glfwGetWindowMonitor(rd.rdWindow) == nullptr) {
            glfwGetWindowPos(rd.rdWindow, &mWindowedPosX, &mWindowedPosY);
            glfwGetWindowSize(rd.rdWindow, &mWindowedWidth, &mWindowedHeight);
        }

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(rd.rdWindow, monitor, 0, 0, width, height, mode->refreshRate);

        Logger::log(1, "%s: switched to fullscreen %dx%d\n", __FUNCTION__, width, height);
    }
    else {
        glfwSetWindowMonitor(rd.rdWindow, nullptr,
            mWindowedPosX, mWindowedPosY, width, height, 0);

        Logger::log(1, "%s: switched to windowed %dx%d\n", __FUNCTION__, width, height);
    }

            glfwSwapInterval(rd.rdVSyncIndex);

                                }

void GraphicsManager::applyAnisotropy(OGLRenderData& rd, std::vector<std::shared_ptr<GltfModel>>& models) {
    float amount = anisotropyFromIndex(rd.rdAnisotropyIndex);

    for (auto& model : models) {
        if (model) {
            model->setAnisotropicFiltering(amount);
        }
    }

    Logger::log(1, "%s: anisotropic filtering set to %.0fx\n", __FUNCTION__, amount);
}

void GraphicsManager::applyLanguage(OGLRenderData& rd)
{

    if (rd.rdLanguageIndex == 0) {
        Localization::setLanguage(Lang::RU);
    }
    else if (rd.rdLanguageIndex == 1) {
        Localization::setLanguage(Lang::EN);
    }
}
