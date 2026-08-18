/* Dear ImGui*/
#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <vector>

#include "OGLRenderData.h"
#include "ModelSettings.h"

class UserInterface {
  public:
    void init(OGLRenderData &renderData);
    void drawSystemMenu(OGLRenderData& renderData, bool& showMenu);
    void createFrame(OGLRenderData& renderData, ModelSettings& settings, bool mShowMenu, bool debug);
    void render();
    void cleanup();
    void drawDeveloperUI(OGLRenderData& renderData, ModelSettings& settings);
    void drawPlayerHUD(OGLRenderData& renderData, ModelSettings& settings);
    inline static bool showSettingsWindow = false;
    inline static bool showTentacle = false;
    inline static bool showEmit = true;
    
    static void drawSettingsOverlay(OGLRenderData& renderData, bool& showSettings);
  private:
    float mFramesPerSecond = 0.0f;
    /* averaging speed */
    float mAveragingAlpha = 0.96f;
    int styleOPT = 0;
    std::vector<float> mFPSValues{};
    int mNumFPSValues = 90;

    std::vector<float> mFrameTimeValues{};
    int mNumFrameTimeValues = 90;

    std::vector<float> mModelUploadValues{};
    int mNumModelUploadValues = 90;

    std::vector<float> mMatrixGenerationValues{};
    int mNumMatrixGenerationValues = 90;

    std::vector<float> mIKValues{};
    int mNumIKValues = 90;

    std::vector<float> mMatrixUploadValues{};
    int mNumMatrixUploadValues = 90;

    std::vector<float> mUiGenValues{};
    int mNumUiGenValues = 90;

    std::vector<float> mUiDrawValues{};
    int mNumUiDrawValues = 90;

    std::vector<std::string> Language = { "Russian", "English" };
   
};
