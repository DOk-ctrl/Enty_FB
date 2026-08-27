#pragma once

#include <vector>
#include <memory>

struct GLFWwindow; 
class GltfModel;
struct OGLRenderData;


class GraphicsManager {
public:
        void update(OGLRenderData& rd, std::vector<std::shared_ptr<GltfModel>>& models);

private:
    void applyVSync(OGLRenderData& rd);
    void applyWindowMode(OGLRenderData& rd);
    void applyAnisotropy(OGLRenderData& rd, std::vector<std::shared_ptr<GltfModel>>& models);
    void applyLanguage(OGLRenderData& rd);
    static void resolutionFromIndex(int index, int& width, int& height);
    static float anisotropyFromIndex(int index);

        int mLastVSync = -1;
    int mLastFullscreen = -1;
    int mLastResolution = -1;
    int mLastAnisotropy = -1;
    int mLastLanguage = -1;

        int mWindowedPosX = 100;
    int mWindowedPosY = 100;
    int mWindowedWidth = 1280;
    int mWindowedHeight = 720;
};
