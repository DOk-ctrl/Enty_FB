#define GLM_ENABLE_EXPERIMENTAL
#include <string>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "UserInterface.h"
#include "ModelSettings.h"
#include "Localization.h"

bool UIsetting = false;

void style() {
    if (UIsetting == false) {
        ImGuiStyle* style = &ImGui::GetStyle(); // near Nier Automata option
        {
            style->WindowPadding = ImVec2(10.0f, 10.0f);
            style->WindowRounding = 5.0f;
            style->ChildRounding = 5.0f;
            style->FramePadding = ImVec2(5.0f, 4.0f);
            style->FrameRounding = 5.0f;
            style->ItemSpacing = ImVec2(5.0f, 5.0f);
            style->ItemInnerSpacing = ImVec2(10.0f, 10.0f);
            style->IndentSpacing = 15.0f;
            style->ScrollbarSize = 16.0f;
            style->ScrollbarRounding = 5.0f;
            style->GrabMinSize = 7.0f;
            style->GrabRounding = 2.0f;
            style->Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
            style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
            style->Colors[ImGuiCol_WindowBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            style->Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 1.00f);
            style->Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.80f);
            style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            style->Colors[ImGuiCol_FrameBg] = ImVec4(0.71f, 0.71f, 0.71f, 0.39f);
            style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.00f, 0.59f, 0.80f, 0.43f);
            style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.00f, 0.47f, 0.71f, 0.67f);
            style->Colors[ImGuiCol_TitleBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.80f);
            style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.78f, 0.78f, 0.78f, 0.39f);
            style->Colors[ImGuiCol_TitleBgActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
            style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
            style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
            style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.59f);
            style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.78f);
            style->Colors[ImGuiCol_PopupBg] = ImVec4(0.78f, 0.78f, 0.78f, 0.98f);
            style->Colors[ImGuiCol_CheckMark] = ImVec4(0.27f, 0.59f, 0.75f, 1.00f);
            style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 0.00f, 0.00f, 0.35f);
            style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.59f);
            style->Colors[ImGuiCol_Button] = ImVec4(0.00f, 0.00f, 0.00f, 0.27f);
            style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.00f, 0.59f, 0.80f, 0.43f);
            style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.47f, 0.71f, 0.67f);
            style->Colors[ImGuiCol_Header] = ImVec4(0.71f, 0.71f, 0.71f, 0.39f);
            style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.51f, 0.67f, 1.00f);
            style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.08f, 0.39f, 0.55f, 1.00f);
            style->Colors[ImGuiCol_Separator] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
            style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.27f, 0.59f, 0.75f, 1.00f);
            style->Colors[ImGuiCol_SeparatorActive] = ImVec4(0.08f, 0.39f, 0.55f, 1.00f);
            style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.78f);
            style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.27f, 0.59f, 0.75f, 0.78f);
            style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.08f, 0.39f, 0.55f, 0.78f);
            style->Colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
            style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
            style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
            style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.27f, 0.59f, 0.75f, 1.00f);
            style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.35f);
        }
    }
    else {
        ImGui::StyleColorsDark();
        }
    }


void UserInterface::init(OGLRenderData &renderData) {
  IMGUI_CHECKVERSION();

  ImGui::CreateContext();

  ImGui_ImplGlfw_InitForOpenGL(renderData.rdWindow, true);

  const char *glslVersion = "#version 460 core";
  ImGui_ImplOpenGL3_Init(glslVersion);
  // загружаем русский язык
  ImGuiIO& io = ImGui::GetIO();

  // Загружаем шрифт с поддержкой кириллицы
  ImFontConfig font_config;
  font_config.OversampleH = 1;
  font_config.OversampleV = 1;
  font_config.PixelSnapH = true;

  // Указываем диапазон символов для кириллицы
  static const ImWchar cyrillic_ranges[] = {
      0x0020, 0x00FF, // Basic Latin + Latin Supplement
      0x0400, 0x044F, // Cyrillic
      0,
  };

 
  io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/Arial.ttf", 16.0f, &font_config, cyrillic_ranges);
 
  // [1] Вытянутый шрифт Rajdhani для YOU DIED (размер 64px)
  io.Fonts->AddFontFromFileTTF("assets/fonts/Rajdhani/Rajdhani-Bold.ttf", 120.0f);
  io.Fonts->AddFontFromFileTTF("assets/fonts/Monomaniac_One/MonomaniacOne-Regular.ttf", 36.0f);
  // [3] ExtraBold с поддержкой КИРИЛЛИЦЫ для кнопок в стиле MiSide
  io.Fonts->AddFontFromFileTTF("assets/fonts/ExtraBold.ttf", 22.0f, &font_config, cyrillic_ranges);
  // конец кода с добавлением кириллицы
  style();
  /* init plot vectors */
  //24.01
  
  mFPSValues.resize(mNumFPSValues);
  mFrameTimeValues.resize(mNumFrameTimeValues);
  mModelUploadValues.resize(mNumModelUploadValues);
  mMatrixGenerationValues.resize(mNumMatrixGenerationValues);
  mIKValues.resize(mNumIKValues);
  mMatrixUploadValues.resize(mNumMatrixUploadValues);
  mUiGenValues.resize(mNumUiGenValues);
  mUiDrawValues.resize(mNumUiDrawValues);
  
}
// 15.01.26
void UserInterface::drawDeveloperUI(OGLRenderData& renderData, ModelSettings& settings) {
    {
        ImGuiWindowFlags imguiWindowFlags = 0;
        //imguiWindowFlags |= ImGuiWindowFlags_NoCollapse;
        //imguiWindowFlags |= ImGuiWindowFlags_NoResize;
        //imguiWindowFlags |= ImGuiWindowFlags_NoMove;

        ImGui::SetNextWindowBgAlpha(0.8f);
        const char* title = "Информация";
        const char* text = "Вы можете переключить на UI персонажа кнопкой F1(нажмите пробел чтобы убавить здоровье)";
        const char* text1 = "Вы можете нажать кнопку Esc, чтобы вызвать контекстное меню";
        const char* text2 = "Нажмите ПКМ(Правая кнопка мыши), чтобы управлять камерой";
        const char* text3 = "WASD - управление";
        ImGui::Begin(title, nullptr, imguiWindowFlags);
        //24.01
        ImGui::Text(text);
        
        ImGui::Text(text1);
        ImGui::Text(text2);
        ImGui::Text(text3);
        
        static float newFps = 0.0f;
        // avoid inf values (division by zero) 
        if (renderData.rdFrameTime > 0.0) {
            newFps = 1.0f / renderData.rdFrameTime * 1000.f;
        }
        // make an averge value to avoid jumps 
        mFramesPerSecond = (mAveragingAlpha * mFramesPerSecond) + (1.0f - mAveragingAlpha) * newFps;

        // clamp manual input on all sliders to min/max 
        ImGuiSliderFlags flags = ImGuiSliderFlags_ClampOnInput;

        static double updateTime = 0.0;

        // avoid literal double compares 
        if (updateTime < 0.000001) {
            updateTime = ImGui::GetTime();
        }

        static int fpsOffset = 0;
        static int frameTimeOffset = 0;
        static int modelUploadOffset = 0;
        static int matrixGenOffset = 0;
        static int ikOffset = 0;
        static int matrixUploadOffset = 0;
        static int uiGenOffset = 0;
        static int uiDrawOffset = 0;
        
        
        while (updateTime < ImGui::GetTime()) {
            mFPSValues.at(fpsOffset) = mFramesPerSecond;
            fpsOffset = ++fpsOffset % mNumFPSValues;

            mFrameTimeValues.at(frameTimeOffset) = renderData.rdFrameTime;
            frameTimeOffset = ++frameTimeOffset % mNumFrameTimeValues;

            mModelUploadValues.at(modelUploadOffset) = renderData.rdUploadToVBOTime;
            modelUploadOffset = ++modelUploadOffset % mNumModelUploadValues;

            mMatrixGenerationValues.at(matrixGenOffset) = renderData.rdMatrixGenerateTime;
            matrixGenOffset = ++matrixGenOffset % mNumMatrixGenerationValues;

            mIKValues.at(ikOffset) = renderData.rdIKTime;
            ikOffset = ++ikOffset % mNumIKValues;

            mMatrixUploadValues.at(matrixUploadOffset) = renderData.rdUploadToUBOTime;
            matrixUploadOffset = ++matrixUploadOffset % mNumMatrixUploadValues;

            mUiGenValues.at(uiGenOffset) = renderData.rdUIGenerateTime;
            uiGenOffset = ++uiGenOffset % mNumUiGenValues;

            mUiDrawValues.at(uiDrawOffset) = renderData.rdUIDrawTime;
            uiDrawOffset = ++uiDrawOffset % mNumUiDrawValues;

            updateTime += 1.0 / 30.0;
        }

        ImGui::BeginGroup();
        ImGui::Text("FPS:");
        ImGui::SameLine();
        ImGui::Text("%s", std::to_string(mFramesPerSecond).c_str());
        ImGui::EndGroup();

        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            float averageFPS = 0.0f;
            for (const auto value : mFPSValues) {
                averageFPS += value;
            }
            averageFPS /= static_cast<float>(mNumFPSValues);
            std::string fpsOverlay = "now:     " + std::to_string(mFramesPerSecond) + "\n30s avg: " + std::to_string(averageFPS);
            ImGui::Text("FPS");
            ImGui::SameLine();
            ImGui::PlotLines("##FrameTimes", mFPSValues.data(), mFPSValues.size(), fpsOffset, fpsOverlay.c_str(), 0.0f, FLT_MAX,
                ImVec2(0, 80));
            ImGui::EndTooltip();
        }

        if (ImGui::CollapsingHeader("Info")) {
            ImGui::Text("Triangles:");
            ImGui::SameLine();
            ImGui::Text("%s", std::to_string(renderData.rdTriangleCount + renderData.rdGltfTriangleCount).c_str());

            ImGui::Checkbox("Draw dark style", &UIsetting);
            style();
            std::string windowDims = std::to_string(renderData.rdWidth) + "x" + std::to_string(renderData.rdHeight);
            ImGui::Text("Window Dimensions:");
            ImGui::SameLine();
            ImGui::Text("%s", windowDims.c_str());

            std::string imgWindowPos = std::to_string(static_cast<int>(ImGui::GetWindowPos().x)) + "/" + std::to_string(static_cast<int>(ImGui::GetWindowPos().y));
            ImGui::Text("ImGui Window Position:");
            ImGui::SameLine();
            ImGui::Text("%s", imgWindowPos.c_str());
        }


        if (ImGui::CollapsingHeader("Timers")) {
            ImGui::BeginGroup();
            ImGui::Text("Frame Time:");
            ImGui::SameLine();
            ImGui::Text("%s", std::to_string(renderData.rdFrameTime).c_str());
            ImGui::SameLine();
            ImGui::Text("ms");
            ImGui::EndGroup();

            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                float averageFrameTime = 0.0f;
                for (const auto value : mFrameTimeValues) {
                    averageFrameTime += value;
                }
                averageFrameTime /= static_cast<float>(mNumMatrixGenerationValues);
                std::string frameTimeOverlay = "now:     " + std::to_string(renderData.rdFrameTime)
                    + " ms\n30s avg: " + std::to_string(averageFrameTime) + " ms";
                ImGui::Text("Frame Time       ");
                ImGui::SameLine();
                ImGui::PlotLines("##FrameTime", mFrameTimeValues.data(), mFrameTimeValues.size(), frameTimeOffset,
                    frameTimeOverlay.c_str(), 0.0f, FLT_MAX, ImVec2(0, 80));
                ImGui::EndTooltip();
            }

            ImGui::BeginGroup();
            ImGui::Text("Model Upload Time:");
            ImGui::SameLine();
            ImGui::Text("%s", std::to_string(renderData.rdUploadToVBOTime).c_str());
            ImGui::SameLine();
            ImGui::Text("ms");
            ImGui::EndGroup();

            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                float averageModelUpload = 0.0f;
                for (const auto value : mModelUploadValues) {
                    averageModelUpload += value;
                }
                averageModelUpload /= static_cast<float>(mNumModelUploadValues);
                std::string modelUploadOverlay = "now:     " + std::to_string(renderData.rdUploadToVBOTime)
                    + " ms\n30s avg: " + std::to_string(averageModelUpload) + " ms";
                ImGui::Text("VBO Upload");
                ImGui::SameLine();
                ImGui::PlotLines("##ModelUploadTimes", mModelUploadValues.data(), mModelUploadValues.size(), modelUploadOffset,
                    modelUploadOverlay.c_str(), 0.0f, FLT_MAX, ImVec2(0, 80));
                ImGui::EndTooltip();
            }

            ImGui::BeginGroup();
            ImGui::Text("Matrix Generation Time:");
            ImGui::SameLine();
            ImGui::Text("%s", std::to_string(renderData.rdMatrixGenerateTime).c_str());
            ImGui::SameLine();
            ImGui::Text("ms");
            ImGui::EndGroup();

            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                float averageMatGen = 0.0f;
                for (const auto value : mMatrixGenerationValues) {
                    averageMatGen += value;
                }
                averageMatGen /= static_cast<float>(mNumMatrixGenerationValues);
                std::string matrixGenOverlay = "now:     " + std::to_string(renderData.rdMatrixGenerateTime)
                    + " ms\n30s avg: " + std::to_string(averageMatGen) + " ms";
                ImGui::Text("Matrix Generation");
                ImGui::SameLine();
                ImGui::PlotLines("##MatrixGenTimes", mMatrixGenerationValues.data(), mMatrixGenerationValues.size(), matrixGenOffset,
                    matrixGenOverlay.c_str(), 0.0f, FLT_MAX, ImVec2(0, 80));
                ImGui::EndTooltip();
            }

            ImGui::BeginGroup();
            ImGui::Text("(IK Generation Time)  :");
            ImGui::SameLine();
            ImGui::Text("%s", std::to_string(renderData.rdIKTime).c_str());
            ImGui::SameLine();
            ImGui::Text("ms");
            ImGui::EndGroup();

            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                float averageIKTime = 0.0f;
                for (const auto value : mIKValues) {
                    averageIKTime += value;
                }
                averageIKTime /= static_cast<float>(mNumIKValues);
                std::string ikOverlay = "now:     " + std::to_string(renderData.rdIKTime)
                    + " ms\n30s avg: " + std::to_string(averageIKTime) + " ms";
                ImGui::Text("(IK Generation)");
                ImGui::SameLine();
                ImGui::PlotLines("##IKTimes", mIKValues.data(), mIKValues.size(), ikOffset,
                    ikOverlay.c_str(), 0.0f, FLT_MAX, ImVec2(0, 80));
                ImGui::EndTooltip();
            }

            ImGui::BeginGroup();
            ImGui::Text("Matrix Upload Time:");
            ImGui::SameLine();
            ImGui::Text("%s", std::to_string(renderData.rdUploadToUBOTime).c_str());
            ImGui::SameLine();
            ImGui::Text("ms");
            ImGui::EndGroup();

            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                float averageMatrixUpload = 0.0f;
                for (const auto value : mMatrixUploadValues) {
                    averageMatrixUpload += value;
                }
                averageMatrixUpload /= static_cast<float>(mNumMatrixUploadValues);
                std::string matrixUploadOverlay = "now:     " + std::to_string(renderData.rdUploadToVBOTime)
                    + " ms\n30s avg: " + std::to_string(averageMatrixUpload) + " ms";
                ImGui::Text("UBO Upload");
                ImGui::SameLine();
                ImGui::PlotLines("##MatrixUploadTimes", mMatrixUploadValues.data(), mMatrixUploadValues.size(), matrixUploadOffset,
                    matrixUploadOverlay.c_str(), 0.0f, FLT_MAX, ImVec2(0, 80));
                ImGui::EndTooltip();
            }

            ImGui::BeginGroup();
            ImGui::Text("UI Generation Time:");
            ImGui::SameLine();
            ImGui::Text("%s", std::to_string(renderData.rdUIGenerateTime).c_str());
            ImGui::SameLine();
            ImGui::Text("ms");
            ImGui::EndGroup();

            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                float averageUiGen = 0.0f;
                for (const auto value : mUiGenValues) {
                    averageUiGen += value;
                }
                averageUiGen /= static_cast<float>(mNumUiGenValues);
                std::string uiGenOverlay = "now:     " + std::to_string(renderData.rdUIGenerateTime)
                    + " ms\n30s avg: " + std::to_string(averageUiGen) + " ms";
                ImGui::Text("UI Generation");
                ImGui::SameLine();
                ImGui::PlotLines("##ModelUpload", mUiGenValues.data(), mUiGenValues.size(), uiGenOffset,
                    uiGenOverlay.c_str(), 0.0f, FLT_MAX, ImVec2(0, 80));
                ImGui::EndTooltip();
            }

            ImGui::BeginGroup();
            ImGui::Text("UI Draw Time:");
            ImGui::SameLine();
            ImGui::Text("%s", std::to_string(renderData.rdUIDrawTime).c_str());
            ImGui::SameLine();
            ImGui::Text("ms");
            ImGui::EndGroup();

            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                float averageUiDraw = 0.0f;
                for (const auto value : mUiDrawValues) {
                    averageUiDraw += value;
                }
                averageUiDraw /= static_cast<float>(mNumUiDrawValues);
                std::string uiDrawOverlay = "now:     " + std::to_string(renderData.rdUIDrawTime)
                    + " ms\n30s avg: " + std::to_string(averageUiDraw) + " ms";
                ImGui::Text("UI Draw");
                ImGui::SameLine();
                ImGui::PlotLines("##UIDrawTimes", mUiDrawValues.data(), mUiDrawValues.size(), uiGenOffset,
                    uiDrawOverlay.c_str(), 0.0f, FLT_MAX, ImVec2(0, 80));
                ImGui::EndTooltip();
            }
        }
        // ================= ДОБАВЛЯЕМ СЮДА =================
        if (ImGui::CollapsingHeader("Graphics Settings")) {
            const char* qualities[] = { "Ultra (Full Res)", "High (Skip 1 Mip)", "Medium (Skip 2 Mips)", "Low (Potato)" };

            ImGui::Text("Texture Quality:");
            ImGui::SameLine();

            // Если игрок меняет значение в комбобоксе, оно автоматически запишется в renderData.rdTextureQuality
            ImGui::PushItemWidth(200);
            ImGui::Combo("##TexQuality", &renderData.rdTextureQuality, qualities, IM_ARRAYSIZE(qualities));
            ImGui::PopItemWidth();

            ImGui::TextDisabled("Changes texture memory sampling instantly.");
        }
        
        // ===================================================
        if (ImGui::CollapsingHeader("Camera")) {
            ImGui::Text("Camera Position:");
            ImGui::SameLine();
            ImGui::Text("%s", glm::to_string(renderData.rdCameraWorldPosition).c_str());

            ImGui::Text("View Azimuth:");
            ImGui::SameLine();
            ImGui::Text("%s", std::to_string(renderData.rdViewAzimuth).c_str());

            ImGui::Text("View Elevation:");
            ImGui::SameLine();
            ImGui::Text("%s", std::to_string(renderData.rdViewElevation).c_str());

            ImGui::Text("Field of View");
            ImGui::SameLine();
            ImGui::SliderInt("##FOV", &renderData.rdFieldOfView, 40, 150, "%d", flags);
        }

        if (ImGui::CollapsingHeader("Lighting (Toy Shader)")) {
            ImGui::TextColored(ImVec4(0.8f, 0.3f, 0.0f, 1.0f), "--- Sun (Directional) ---");
            ImGui::SliderFloat3("Sun Direction", glm::value_ptr(renderData.rdSunDirection), -1.0f, 1.0f, "%.2f", flags);
            ImGui::ColorEdit3("Sun Color", glm::value_ptr(renderData.rdSunColor));
            ImGui::SliderFloat("Sun Intensity", &renderData.rdSunIntensity, 0.0f, 5.0f, "%.2f", flags);

            ImGui::Separator();

            ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "--- Point Light (Bulb) ---");
            ImGui::SliderFloat3("Light Pos", glm::value_ptr(renderData.rdLightPos), -50.0f, 50.0f, "%.1f", flags);
            ImGui::ColorEdit3("Light Color", glm::value_ptr(renderData.rdPointColor));
            ImGui::SliderFloat("Light Intensity", &renderData.rdPointIntensity, 0.0f, 10.0f, "%.2f", flags);
            ImGui::SliderFloat("Attenuation", &renderData.rdPointAttenuation, 0.001f, 1.0f, "%.3f", flags);
        }

        if (ImGui::CollapsingHeader("glTF Instances")) {
            ImGui::Text("Model Instances  : %d", renderData.rdNumberOfInstances);

            ImGui::Text("Selected Instance:");
            ImGui::SameLine();
            ImGui::PushButtonRepeat(true);
            if (ImGui::ArrowButton("##LEFT", ImGuiDir_Left) &&
                renderData.rdCurrentSelectedInstance > 0) {
                renderData.rdCurrentSelectedInstance--;
            }
            ImGui::SameLine();
            ImGui::PushItemWidth(30);
            ImGui::DragInt("##SELINST", &renderData.rdCurrentSelectedInstance, 1, 0,
                renderData.rdNumberOfInstances - 1, "%3d", flags);
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::ArrowButton("##RIGHT", ImGuiDir_Right) &&
                renderData.rdCurrentSelectedInstance < (renderData.rdNumberOfInstances - 1)) {
                renderData.rdCurrentSelectedInstance++;
            }
            ImGui::PopButtonRepeat();
            
            ImGui::Text("World Pos (X/Z)  :");
            ImGui::SameLine();
            ImGui::SliderFloat3("##WORLDPOS", glm::value_ptr(settings.msWorldPosition),
                -550.0f, 550.0f, "%.1f", flags);

            ImGui::Text("World Rotation   :");
            ImGui::SameLine();
            ImGui::SliderFloat("##WORLDROT", &settings.msWorldRotation.y,
                -180.0f, 180.0f, "%.0f", flags);
                
        
        
        }
        if (ImGui::CollapsingHeader("glTF Model")) {
            ImGui::Checkbox("Draw Model", &settings.msDrawModel);
            ImGui::Checkbox("Draw Skeleton", &settings.msDrawSkeleton);

            ImGui::Text("Vertex Skinning:");
            ImGui::SameLine();
            if (ImGui::RadioButton("Linear",
                settings.msVertexSkinningMode == skinningMode::linear)) {
                settings.msVertexSkinningMode = skinningMode::linear;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Dual Quaternion",
                settings.msVertexSkinningMode == skinningMode::dualQuat)) {
                settings.msVertexSkinningMode = skinningMode::dualQuat;
            }
        }

        if (ImGui::CollapsingHeader("glTF Animation")) {
            ImGui::Checkbox("Play Animation", &settings.msPlayAnimation);

            if (!settings.msPlayAnimation) {
                ImGui::BeginDisabled();
            }

            ImGui::Text("Animation Direction:");
            ImGui::SameLine();
            if (ImGui::RadioButton("Forward",
                settings.msAnimationPlayDirection == replayDirection::forward)) {
                settings.msAnimationPlayDirection = replayDirection::forward;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Backward",
                settings.msAnimationPlayDirection == replayDirection::backward)) {
                settings.msAnimationPlayDirection = replayDirection::backward;
            }

            if (!settings.msPlayAnimation) {
                ImGui::EndDisabled();
            }

            ImGui::Text("Clip   ");
            ImGui::SameLine();
            if (ImGui::BeginCombo("##ClipCombo",
                settings.msClipNames.at(settings.msAnimClip).c_str())) {
                for (int i = 0; i < settings.msClipNames.size(); ++i) {
                    const bool isSelected = (settings.msAnimClip == i);
                    if (ImGui::Selectable(settings.msClipNames.at(i).c_str(), isSelected)) {
                        settings.msAnimClip = i;
                    }

                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            if (settings.msPlayAnimation) {
                ImGui::Text("Speed  ");
                ImGui::SameLine();
                ImGui::SliderFloat("##ClipSpeed", &settings.msAnimSpeed, 0.0f, 2.0f, "%.3f", flags);
            }
            else {
                ImGui::Text("Timepos");
                ImGui::SameLine();
                ImGui::SliderFloat("##ClipPos", &settings.msAnimTimePosition, 0.0f,
                    settings.msAnimEndTime, "%.3f", flags);
            }
        }

        if (ImGui::CollapsingHeader("glTF Animation Blending")) {
            ImGui::Text("Blending Type:");
            ImGui::SameLine();
            if (ImGui::RadioButton("Fade In/Out",
                settings.msBlendingMode == blendMode::fadeinout)) {
                settings.msBlendingMode = blendMode::fadeinout;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Crossfading",
                settings.msBlendingMode == blendMode::crossfade)) {
                settings.msBlendingMode = blendMode::crossfade;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Additive",
                settings.msBlendingMode == blendMode::additive)) {
                settings.msBlendingMode = blendMode::additive;
            }

            if (settings.msBlendingMode == blendMode::fadeinout) {
                ImGui::Text("Blend Factor");
                ImGui::SameLine();
                ImGui::SliderFloat("##BlendFactor", &settings.msAnimBlendFactor, 0.0f, 1.0f, "%.3f",
                    flags);
            }

            if (settings.msBlendingMode == blendMode::crossfade ||
                settings.msBlendingMode == blendMode::additive) {
                ImGui::Text("Dest Clip   ");
                ImGui::SameLine();
                if (ImGui::BeginCombo("##DestClipCombo",
                    settings.msClipNames.at(settings.msCrossBlendDestAnimClip).c_str())) {
                    for (int i = 0; i < settings.msClipNames.size(); ++i) {
                        const bool isSelected = (settings.msCrossBlendDestAnimClip == i);
                        if (ImGui::Selectable(settings.msClipNames.at(i).c_str(), isSelected)) {
                            settings.msCrossBlendDestAnimClip = i;
                        }

                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::Text("Cross Blend ");
                ImGui::SameLine();
                ImGui::SliderFloat("##CrossBlendFactor", &settings.msAnimCrossBlendFactor, 0.0f, 1.0f,
                    "%.3f", flags);
            }

            if (settings.msBlendingMode == blendMode::additive) {
                ImGui::Text("Split Node  ");
                ImGui::SameLine();
                if (ImGui::BeginCombo("##SplitNodeCombo",
                    settings.msSkelNodeNames.at(settings.msSkelSplitNode).c_str())) {
                    for (int i = 0; i < settings.msSkelNodeNames.size(); ++i) {
                        if (settings.msSkelNodeNames.at(i).compare("(invalid)") != 0) {
                            const bool isSelected = (settings.msSkelSplitNode == i);
                            if (ImGui::Selectable(settings.msSkelNodeNames.at(i).c_str(), isSelected)) {
                                settings.msSkelSplitNode = i;
                            }

                            if (isSelected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                    }
                    ImGui::EndCombo();
                }
            }
        }

        if (ImGui::CollapsingHeader("glTF Inverse Kinematic")) {
            ImGui::Text("Inverse Kinematics");
            ImGui::SameLine();
            if (ImGui::RadioButton("Off",
                settings.msIkMode == ikMode::off)) {
                settings.msIkMode = ikMode::off;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("CCD",
                settings.msIkMode == ikMode::ccd)) {
                settings.msIkMode = ikMode::ccd;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("FABRIK",
                settings.msIkMode == ikMode::fabrik)) {
                settings.msIkMode = ikMode::fabrik;
            }

            if (settings.msIkMode == ikMode::ccd ||
                settings.msIkMode == ikMode::fabrik) {
                ImGui::Text("IK Iterations  :");
                ImGui::SameLine();
                ImGui::SliderInt("##IKITER", &settings.msIkIterations, 0, 15, "%d", flags);

                ImGui::Text("Target Position:");
                ImGui::SameLine();
                ImGui::SliderFloat3("##IKTargetPOS", glm::value_ptr(settings.msIkTargetPos), -10.0f,
                    10.0f, "%.3f", flags);
                ImGui::Text("Effector Node  :");
                ImGui::SameLine();
                if (ImGui::BeginCombo("##EffectorNodeCombo",
                    settings.msSkelNodeNames.at(settings.msIkEffectorNode).c_str())) {
                    for (int i = 0; i < settings.msSkelNodeNames.size(); ++i) {
                        if (settings.msSkelNodeNames.at(i).compare("(invalid)") != 0) {
                            const bool isSelected = (settings.msIkEffectorNode == i);
                            if (ImGui::Selectable(settings.msSkelNodeNames.at(i).c_str(), isSelected)) {
                                settings.msIkEffectorNode = i;
                            }

                            if (isSelected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::Text("IK Root Node   :");
                ImGui::SameLine();
                if (ImGui::BeginCombo("##RootNodeCombo",
                    settings.msSkelNodeNames.at(settings.msIkRootNode).c_str())) {
                    for (int i = 0; i < settings.msSkelNodeNames.size(); ++i) {
                        if (settings.msSkelNodeNames.at(i).compare("(invalid)") != 0) {
                            const bool isSelected = (settings.msIkRootNode == i);
                            if (ImGui::Selectable(settings.msSkelNodeNames.at(i).c_str(), isSelected)) {
                                settings.msIkRootNode = i;
                            }

                            if (isSelected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                    }
                    ImGui::EndCombo();
                }
            }
        }
        
        ImGui::End();
    }

}
void UserInterface::drawPlayerHUD(OGLRenderData& renderData, ModelSettings& settings) {
    ImVec2 screenSize = ImGui::GetIO().DisplaySize;

    ImVec4 nierColorText = ImVec4(0.72f, 0.70f, 0.60f, 0.9f);
    ImU32 nierColorBar = IM_COL32(160, 155, 130, 255);

    // -------------------------------------------------------------------
    // 1. ХП-БАР (Левый верхний угол)
    // -------------------------------------------------------------------
    float maxHP = renderData.rdPlayerMaxHP;
    float currentHP = renderData.rdPlayerHP;
    static float displayedHP = renderData.rdPlayerMaxHP;

    if (displayedHP > currentHP) {
        displayedHP -= 0.1f;
        if (displayedHP < currentHP) displayedHP = currentHP;
    }
    else {
        displayedHP = currentHP;
    }

    float totalBarWidth = screenSize.x * 0.15f;
    float barHeight = 7.0f;

    ImGui::SetNextWindowSize(ImVec2(totalBarWidth + 100, 100));
    ImGui::SetNextWindowPos(ImVec2(50, 50));

    if (ImGui::Begin("NierHUD", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, nierColorBar);
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();

        drawList->AddRectFilled(p, ImVec2(p.x + totalBarWidth, p.y + barHeight), IM_COL32(40, 40, 40, 200));

        float animWidth = totalBarWidth * (displayedHP / maxHP);
        drawList->AddRectFilled(p, ImVec2(p.x + animWidth, p.y + barHeight), IM_COL32(100, 100, 100, 255));

        float currentWidth = totalBarWidth * (currentHP / maxHP);
        drawList->AddRectFilled(p, ImVec2(p.x + currentWidth, p.y + barHeight), IM_COL32(235, 235, 220, 255));

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + barHeight + 8);
        ImGui::Text("  HP: %.0f / %.0f", currentHP, maxHP);
        ImGui::PopStyleColor();
        ImGui::End();
    }

    // -------------------------------------------------------------------
    // 2. КВЕСТ-ТРЕКЕР / ЦЕЛЬ (Правый верхний угол)
    // -------------------------------------------------------------------
    float questWidth = 340.0f;
    float rightPadding = 40.0f;
    float topPadding = 45.0f;

    ImGui::SetNextWindowPos(ImVec2(screenSize.x - rightPadding, topPadding), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(questWidth, 0.0f));

    if (ImGui::Begin("QuestHUD", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // Динамический выбор палитры в зависимости от состояния игры
        ImVec4 headerTextColor;
        ImU32 headerLineColor;

        if (renderData.rdIsGameWon) {
            // Изумрудно-зеленая гамма победы
            headerTextColor = ImVec4(0.25f, 0.88f, 0.42f, 1.0f);
            headerLineColor = IM_COL32(50, 210, 100, 230);
        }
        else {
            // Стандартная золотая РПГ гамма
            headerTextColor = ImVec4(0.98f, 0.80f, 0.25f, 1.0f);
            headerLineColor = IM_COL32(240, 185, 45, 220);
        }

        ImU32 shadowLineColor = IM_COL32(10, 10, 10, 180);

        // Заголовок цели
        ImGui::PushStyleColor(ImGuiCol_Text, headerTextColor);
        if (renderData.rdIsGameWon) {
            ImGui::Text(Localization::get("goalT").c_str());
        }
        else {
            ImGui::Text(Localization::get("goal").c_str());
        }
        ImGui::PopStyleColor();

        // Разделительная линия
        ImVec2 lineStart = ImGui::GetCursorScreenPos();
        lineStart.y += 4.0f;
        ImVec2 lineEnd = ImVec2(lineStart.x + questWidth, lineStart.y);

        drawList->AddLine(ImVec2(lineStart.x, lineStart.y + 1.5f), ImVec2(lineEnd.x, lineEnd.y + 1.5f), shadowLineColor, 2.0f);
        drawList->AddLine(lineStart, lineEnd, headerLineColor, 2.0f);

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);

        // Текст задачи / Благодарность
        if (renderData.rdIsGameWon) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.95f, 0.88f, 0.95f));
            ImGui::Text(Localization::get("1.1").c_str());
            ImGui::Text(Localization::get("2.1").c_str());
            ImGui::Text(Localization::get("3.1").c_str());
            ImGui::Text(Localization::get("email").c_str());
            ImGui::PopStyleColor();
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.92f, 0.90f, 0.82f, 0.95f));
            ImGui::Text(Localization::get("14.1").c_str());
            ImGui::Text(Localization::get("15.1").c_str());
            ImGui::PopStyleColor();
        }

        ImGui::End();
    }
}
void UserInterface::drawSettingsOverlay(OGLRenderData& renderData, bool& showSettings)
{
    if (!showSettings) return;

    static int activeSettingTab = 2; // По умолчанию Графика

    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiStyle oldStyle = style;

    // --- Настройка стиля под визуал скриншота ---
    style.WindowRounding = 14.0f;
    style.FrameRounding = 10.0f;
    style.PopupRounding = 10.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;

    // Цветовая палитра (Deep Indigo / Neon Blue / Soft Violet)
    ImVec4 bgDark = ImVec4(0.08f, 0.09f, 0.18f, 0.88f); // Полупрозрачный темно-синий фон
    ImVec4 panelBg = ImVec4(0.18f, 0.18f, 0.32f, 0.45f); // Фиолетово-серые плашки
    ImVec4 buttonNormal = ImVec4(0.25f, 0.24f, 0.42f, 0.70f); // Цвет кнопок из главного меню
    ImVec4 buttonHovered = ImVec4(0.38f, 0.35f, 0.65f, 0.90f); // Ярче при наведении
    ImVec4 buttonActive = ImVec4(0.18f, 0.55f, 0.95f, 0.90f); // Неоново-голубой актив
    ImVec4 textColor = ImVec4(0.92f, 0.95f, 1.00f, 0.95f); // Бело-голубой текст
    ImVec4 accentCyan = ImVec4(0.20f, 0.75f, 1.00f, 1.00f); // Неоновый циан

    style.Colors[ImGuiCol_WindowBg] = bgDark;
    style.Colors[ImGuiCol_Border] = ImVec4(0.35f, 0.45f, 0.80f, 0.30f);
    style.Colors[ImGuiCol_Text] = textColor;
    style.Colors[ImGuiCol_Button] = buttonNormal;
    style.Colors[ImGuiCol_ButtonHovered] = buttonHovered;
    style.Colors[ImGuiCol_ButtonActive] = buttonActive;

    // Полноэкранный оверлей с небольшим отступом от краев для ощущения "плавающей карточки"
    float marginX = renderData.rdWidth * 0.05f;
    float marginY = renderData.rdHeight * 0.05f;

    ImGui::SetNextWindowPos(ImVec2(marginX, marginY));
    ImGui::SetNextWindowSize(ImVec2(renderData.rdWidth - marginX * 2.0f, renderData.rdHeight - marginY * 2.0f));

    ImGui::Begin("SETTINGS_SCREEN", &showSettings,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 winPos = ImGui::GetWindowPos();
    ImVec2 winSize = ImGui::GetWindowSize();

    // --- ЗАГОЛОВОК ---
    std::string rawTitle = Localization::get("settings");
    if (rawTitle.empty()) rawTitle = "НАСТРОЙКИ";

    ImGui::SetCursorPos(ImVec2(35.0f, 25.0f));
    ImGui::SetWindowFontScale(1.4f);
    ImGui::TextColored(accentCyan, "%s", rawTitle.c_str());
    ImGui::SetWindowFontScale(1.0f);

    // Разделительная линия с градиентом/своим свечением
    float lineY = winPos.y + 70.0f;
    drawList->AddLine(
        ImVec2(winPos.x + 35.0f, lineY),
        ImVec2(winPos.x + winSize.x - 35.0f, lineY),
        ImGui::ColorConvertFloat4ToU32(ImVec4(0.35f, 0.55f, 0.95f, 0.40f)),
        2.0f
    );

    // Разметка контента
    float contentStartY = 95.0f;
    float leftColWidth = 240.0f;
    float itemHeight = 48.0f;

    // Лямбда для создания стилизованных закругленных кнопок категорий
    auto DrawCategoryTab = [&](const char* locKey, int tabIdx) {
        bool isActive = (activeSettingTab == tabIdx);
        std::string label = Localization::get(locKey);
        if (label.empty()) label = locKey;

        if (isActive) {
            ImGui::PushStyleColor(ImGuiCol_Button, buttonActive);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonActive);
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Button, buttonNormal);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHovered);
        }

        bool pressed = ImGui::Button(label.c_str(), ImVec2(leftColWidth, itemHeight));
        ImGui::PopStyleColor(2);

        if (pressed) {
            activeSettingTab = tabIdx;
        }
        return pressed;
        };

    // --- ЛЕВАЯ ПАНЕЛЬ (КАТЕГОРИИ) ---
    ImGui::SetCursorPos(ImVec2(35.0f, contentStartY));
    ImGui::BeginGroup();

    DrawCategoryTab("language", 0); ImGui::Dummy(ImVec2(0, 8.0f));
    DrawCategoryTab("camera", 1);   ImGui::Dummy(ImVec2(0, 8.0f));
    DrawCategoryTab("grathics", 2); ImGui::Dummy(ImVec2(0, 8.0f));

    ImGui::EndGroup();

    // Вертикальный разделитель между категориями и настройками
    float sepX = winPos.x + 35.0f + leftColWidth + 25.0f;
    drawList->AddLine(
        ImVec2(sepX, winPos.y + contentStartY),
        ImVec2(sepX, winPos.y + winSize.y - 90.0f),
        ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.12f)),
        1.5f
    );

    // --- ПРАВАЯ ПАНЕЛЬ (ОПЦИИ) ---
    float rightColX = leftColWidth + 85.0f;
    float dropdownWidth = 480.0f;

    // Лямбда для кастомных выпадок в стиле овала/закругленной плашки
    auto DrawOptionDropdown = [&](const char* labelKey, int* currentIndex, const char* options[], int optionsCount) {
        ImVec2 screenPos = ImGui::GetCursorScreenPos();
        std::string label = Localization::get(labelKey);
        if (label.empty()) label = labelKey;

        const char* currentValText = options[*currentIndex];

        // Задняя карточка для опции
        drawList->AddRectFilled(
            screenPos,
            ImVec2(screenPos.x + dropdownWidth, screenPos.y + itemHeight),
            ImGui::ColorConvertFloat4ToU32(panelBg),
            10.0f
        );

        // Текст названия настройки
        ImVec2 textPos = ImVec2(screenPos.x + 20.0f, screenPos.y + (itemHeight - ImGui::GetTextLineHeight()) * 0.5f);
        drawList->AddText(textPos, ImGui::ColorConvertFloat4ToU32(textColor), label.c_str());

        // Выпадающий селектор справа
        std::string comboId = std::string("##combo_") + labelKey;
        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + dropdownWidth - 210.0f, ImGui::GetCursorPosY() + (itemHeight - 32.0f) * 0.5f));
        ImGui::SetNextItemWidth(190.0f);

        ImGui::PushStyleColor(ImGuiCol_FrameBg, buttonNormal);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, buttonHovered);
        ImGui::PushStyleColor(ImGuiCol_PopupBg, bgDark);

        if (ImGui::BeginCombo(comboId.c_str(), currentValText, ImGuiComboFlags_HeightRegular)) {
            for (int i = 0; i < optionsCount; i++) {
                bool isSelected = (*currentIndex == i);
                if (ImGui::Selectable(options[i], isSelected)) {
                    *currentIndex = i;
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::PopStyleColor(3);

        // Сдвигаем курсор под элемент
        ImGui::SetCursorScreenPos(ImVec2(screenPos.x, screenPos.y + itemHeight + 10.0f));
        };

    // Отрисовка подменю "Язык"
    if (activeSettingTab == 0) {
        ImGui::SetCursorPos(ImVec2(rightColX, contentStartY));
        ImGui::BeginGroup();

        std::string labelRu = Localization::get("russian"); if (labelRu.empty()) labelRu = "Русский";
        const char* language[] = { labelRu.c_str(), "English" };
        DrawOptionDropdown("language", &renderData.rdLanguageIndex, language, IM_ARRAYSIZE(language));

        ImGui::EndGroup();
    }

    // Отрисовка подменю "Графика"
    if (activeSettingTab == 2) {
        ImGui::SetCursorPos(ImVec2(rightColX, contentStartY));
        ImGui::BeginGroup();

        const char* resolutions[] = { "1280x720", "1920x1080", "2560x1440", "3840x2160" };
        DrawOptionDropdown("resolution", &renderData.rdResolutionIndex, resolutions, IM_ARRAYSIZE(resolutions));

        std::string labelOff = Localization::get("off"); if (labelOff.empty()) labelOff = "Выкл.";
        std::string labelOn = Localization::get("on");   if (labelOn.empty()) labelOn = "Вкл.";
        const char* toggleOptions[] = { labelOff.c_str(), labelOn.c_str() };

        DrawOptionDropdown("fullscreen", &renderData.rdFullscreenIndex, toggleOptions, IM_ARRAYSIZE(toggleOptions));
        DrawOptionDropdown("vsync", &renderData.rdVSyncIndex, toggleOptions, IM_ARRAYSIZE(toggleOptions));

        const char* msaaOptions[] = { labelOff.c_str(), "MSAA 2x", "MSAA 4x", "MSAA 8x" };
        DrawOptionDropdown("antialiasing", &renderData.rdMSAAIndex, msaaOptions, IM_ARRAYSIZE(msaaOptions));

        const char* afOptions[] = { labelOff.c_str(), "AF 2x", "AF 4x", "AF 8x", "AF 16x" };
        DrawOptionDropdown("texture_filtering", &renderData.rdAnisotropyIndex, afOptions, IM_ARRAYSIZE(afOptions));

        std::string labelUltra = Localization::get("ultra");  if (labelUltra.empty())  labelUltra = "Ультра";
        std::string labelHigh = Localization::get("high");   if (labelHigh.empty())   labelHigh = "Высокое";
        std::string labelMedium = Localization::get("medium"); if (labelMedium.empty()) labelMedium = "Среднее";
        std::string labelLow = Localization::get("low");    if (labelLow.empty())    labelLow = "Низкое";

        const char* qualities[] = { labelUltra.c_str(), labelHigh.c_str(), labelMedium.c_str(), labelLow.c_str() };
        DrawOptionDropdown("texture_quality", &renderData.rdTextureQuality, qualities, IM_ARRAYSIZE(qualities));

        ImGui::EndGroup();
    }
    
    // --- НИЖНЯЯ ПАНЕЛЬИ КНОПКА "НАЗАД" ---
    float footerY = winSize.y - 65.0f;
    drawList->AddLine(
        ImVec2(winPos.x + 35.0f, winPos.y + footerY - 15.0f),
        ImVec2(winPos.x + winSize.x - 35.0f, winPos.y + footerY - 15.0f),
        ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.12f)),
        1.0f
    );

    ImGui::SetCursorPos(ImVec2(35.0f, footerY));

    std::string backLabel = Localization::get("back");
    if (backLabel.empty()) backLabel = "Назад";

    if (ImGui::Button(backLabel.c_str(), ImVec2(160.0f, 40.0f))) {
        showSettings = false;
    }

    ImGui::End();

    // Восстанавливаем дефолтный стиль
    style = oldStyle;
}
//15.01.26

void UserInterface::drawSystemMenu(OGLRenderData& renderData, bool& showMenu) {
    if (!showMenu) return;

    // Состояния окон меню
    static int currentSubMenu = 0;   // 0 - Главное меню, 1 - Экран настроек
    static int activeSettingTab = 2; // По умолчанию активна вкладка "Экран"

    ImGuiStyle& style = ImGui::GetStyle();

    // Бэкап дефолтных стилей приложения
    ImVec4 oldWinBg = style.Colors[ImGuiCol_WindowBg];
    ImVec4 oldText = style.Colors[ImGuiCol_Text];

    // Применяем фирменные приглушенные тона NieR
    style.WindowRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.WindowBorderSize = 0.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.95f, 0.94f, 0.886f, 1.0f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.20f, 0.19f, 0.17f, 0.90f);

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)renderData.rdWidth, (float)renderData.rdHeight));

    ImGui::Begin("SYSTEM_SCREEN", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    // ==========================================
    // ВЕРХНЯЯ ПАНЕЛЬ: Вкладка справа сверху
    // ==========================================
    float topMargin = 20.0f;
    float tabWidth = 200.0f;
    float tabHeight = 40.0f;
    float rightMargin = renderData.rdWidth * 0.08f;

    ImGui::SetCursorPos(ImVec2(renderData.rdWidth - rightMargin - tabWidth, topMargin));

    style.Colors[ImGuiCol_Button] = ImVec4(0.20f, 0.19f, 0.17f, 1.0f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.95f, 0.94f, 0.886f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.19f, 0.17f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.19f, 0.17f, 1.0f);

    ImGui::Button(Localization::get("system").c_str(), ImVec2(tabWidth, tabHeight));

    // Возвращаем цвета текста для остальной рабочей области
    style.Colors[ImGuiCol_Text] = ImVec4(0.20f, 0.19f, 0.17f, 0.90f);

    ImGui::SetCursorPosY(topMargin + tabHeight + 10.0f);
    ImGui::Separator();
    ImGui::SetCursorPosY(topMargin + tabHeight + 13.0f);
    ImGui::Separator();

    // ==========================================
    // РАЗМЕРЫ И ОСТУПЫ ДЛЯ КНОПОК И ЛИНИЙ
    // ==========================================
    static float testWidth = 615.926f;
    static float testHeight = 78.424f;
    static float testSpacing = 34.66f;

    static float lineThickness = 9.333f;
    static float thinLineOffset = 12.582f;
    static float thinLineThickness = 2.609f;
    static float lineLengthFactor = 0.65f;

    float leftMargin = renderData.rdWidth * 0.08f;
    float buttonsStartY = renderData.rdHeight * 0.24f;

    // ==========================================
    // 1. ДИНАМИЧЕСКИЙ ЗАГОЛОВОК С ЭФФЕКТОМ ТЕНИ
    // ==========================================
    float titleX = leftMargin;
    float titleY = buttonsStartY - 110.0f;

    // Если зашли в настройки, заголовок меняется на соответствующий
    std::string rawTitle = (currentSubMenu == 1) ? Localization::get("settings") : Localization::get("system");

    if (currentSubMenu != 1) {
        if (!rawTitle.empty() && rawTitle.front() == '[') rawTitle.erase(0, 1);
        if (!rawTitle.empty() && rawTitle.back() == ']') rawTitle.pop_back();
        rawTitle.erase(0, rawTitle.find_first_not_of(" "));
        rawTitle.erase(rawTitle.find_last_not_of(" ") + 1);
    }

    float mainFontScale = 1.0f;
    float shadowFontScale = 1.1f;

    ImGui::SetCursorPos(ImVec2(titleX + 5.0f, titleY + 1.0f));
    ImGui::SetWindowFontScale(shadowFontScale);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.20f, 0.19f, 0.17f, 0.20f));
    ImGui::Text(rawTitle.c_str());
    ImGui::PopStyleColor();

    ImGui::SetCursorPos(ImVec2(titleX, titleY));
    ImGui::SetWindowFontScale(mainFontScale);
    ImGui::Text(rawTitle.c_str());
    ImGui::SetWindowFontScale(1.0f);

    // ==========================================
    // 2. ОТРИСОВКА ВЕРТИКАЛЬНОЙ ДВОЙНОЙ ЛИНИИ
    // ==========================================
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();

    float lineStartX = windowPos.x + 130.0f;
    float lineStartY = windowPos.y + buttonsStartY - 10.0f;
    float lineEndY = windowPos.y + renderData.rdHeight * lineLengthFactor;

    ImU32 lineColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.20f, 0.19f, 0.17f, 0.4f));

    drawList->AddLine(ImVec2(lineStartX, lineStartY), ImVec2(lineStartX, lineEndY), lineColor, lineThickness);
    float thinLineX = lineStartX + thinLineOffset;
    drawList->AddLine(ImVec2(thinLineX, lineStartY), ImVec2(thinLineX, lineEndY), lineColor, thinLineThickness);

    // ==========================================
    // 3. ЛЕВАЯ КОЛОНКА МЕНЮ (Кнопки / Категории)
    // ==========================================
    ImGui::SetCursorPos(ImVec2(lineStartX - windowPos.x + 35.0f, buttonsStartY));
    ImGui::BeginGroup();

    float bW = testWidth;
    float bH = testHeight;

    // Автоматическое сужение кнопок, если находимся внутри меню настроек
    float currentButtonWidth = (currentSubMenu == 1) ? 260.0f : bW;

    // Универсальный лямбда-виджет кнопки (теперь строго работает по ключам локализации)
    auto DrawNieRButton = [&](const char* localizationKey, bool isActiveTab = false) {
        ImVec2 screenPos = ImGui::GetCursorScreenPos();

        // ТЕПЕРЬ ВСЕГДА тащим строку из твоего файла локализации
        std::string label = Localization::get(localizationKey);

        // Если вдруг ключа нет в файле, выведем сам ключ, чтобы код не падал
        if (label.empty()) {
            label = localizationKey;
        }

        // Цвета заднего фона плашек
        if (isActiveTab) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.19f, 0.17f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.19f, 0.17f, 0.8f));
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.19f, 0.17f, 0.1f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.19f, 0.17f, 0.8f));
        }
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.19f, 0.17f, 0.9f));

        // Цвета текста по умолчанию
        if (isActiveTab) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.94f, 0.886f, 1.0f));
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.20f, 0.19f, 0.17f, 0.90f));
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(45.0f, style.FramePadding.y));

        // Вызов кнопки
        bool pressed = ImGui::Button(label.c_str(), ImVec2(currentButtonWidth, bH));
        bool isHovered = ImGui::IsItemHovered() || isActiveTab;

        ImDrawList* wDrawList = ImGui::GetWindowDrawList();

        // Отрисовка светлого текста при наведении + имитация жирного Bold-эффекта
        if (isHovered) {
            ImVec2 textPos = ImVec2(screenPos.x + 45.0f, screenPos.y + (bH - ImGui::GetTextLineHeight()) * 0.5f);
            ImU32 textActiveColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.95f, 0.94f, 0.886f, 1.0f));

            if (!isActiveTab) {
                wDrawList->AddRectFilled(
                    ImVec2(screenPos.x + 40.0f, screenPos.y + 2.0f),
                    ImVec2(screenPos.x + currentButtonWidth - 5.0f, screenPos.y + bH - 2.0f),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(0.2f, 0.19f, 0.17f, 0.8f))
                );
            }

            wDrawList->AddText(textPos, textActiveColor, label.c_str());
            wDrawList->AddText(ImVec2(textPos.x + 0.5f, textPos.y), textActiveColor, label.c_str());
            wDrawList->AddText(ImVec2(textPos.x, textPos.y + 0.5f), textActiveColor, label.c_str());
        }

        // Геометрия квадратного маркера слева
        float squareSize = 14.0f;
        float sqX = screenPos.x + 18.0f;
        float sqY = screenPos.y + (bH - squareSize) * 0.5f;

        ImU32 sqColor = isHovered
            ? ImGui::ColorConvertFloat4ToU32(ImVec4(0.95f, 0.94f, 0.886f, 1.0f))
            : ImGui::ColorConvertFloat4ToU32(ImVec4(0.20f, 0.19f, 0.17f, 0.90f));

        wDrawList->AddRectFilled(ImVec2(sqX, sqY), ImVec2(sqX + squareSize, sqY + squareSize), sqColor);

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4);

        return pressed;
        };

    // Отрисовка левой рабочей зоны (теперь передаем только КЛЮЧИ из твоего файла локализации)
    if (currentSubMenu == 0) {
        // --- ГЛАВНЫЙ ЭКРАН СИСТЕМЫ ---
        if (DrawNieRButton("save")) { /* код сохранения */ } ImGui::Dummy(ImVec2(0, testSpacing));
        if (DrawNieRButton("load")) { /* код загрузки */ }   ImGui::Dummy(ImVec2(0, testSpacing));
        if (DrawNieRButton("settings")) { currentSubMenu = 1; }  ImGui::Dummy(ImVec2(0, testSpacing));
        if (DrawNieRButton("exit")) { glfwSetWindowShouldClose(renderData.rdWindow, true); }
    }
    else if (currentSubMenu == 1) {
        // --- ПОДМЕНЮ НАСТРОЕК (ПАНЕЛЬ КАТЕГОРИЙ) ---
        if (DrawNieRButton("language", activeSettingTab == 0)) { activeSettingTab = 0; } ImGui::Dummy(ImVec2(0, 12.0f));
        if (DrawNieRButton("camera", activeSettingTab == 1)) { activeSettingTab = 1; } ImGui::Dummy(ImVec2(0, 12.0f));
        if (DrawNieRButton("grathics", activeSettingTab == 2)) { activeSettingTab = 2; } ImGui::Dummy(ImVec2(0, 12.0f));

        // ИЗ ЭТОГО МЕСТА СТАРЫЙ СЕПАРАТОР И КНОПКА "НАЗАД" УДАЛЕНЫ!
    }

    ImGui::EndGroup();

    // ==========================================
    // --- ПРАВАЯ ПАНЕЛЬ: ПИСЬМО ОТ КОМАНДИРА ---
    // ==========================================
    if (currentSubMenu == 0) {
        float contentStartY = buttonsStartY;
        ImVec2 winSize = ImGui::GetWindowSize();
        float footerY = winSize.y - 40.0f;

        float letterWidth = 400.0f;
        float letterX = winSize.x - 35.0f - letterWidth;
        float letterY = contentStartY;
        float letterHeight = footerY - contentStartY - 20.0f;

        ImVec2 letterMin = ImVec2(windowPos.x + letterX, windowPos.y + letterY);
        ImVec2 letterMax = ImVec2(letterMin.x + letterWidth, letterMin.y + letterHeight);

        // Цветовая палитра NieR
        ImVec4 panelBg = ImVec4(0.20f, 0.19f, 0.17f, 0.05f); // Едва заметный темный подтон
        ImVec4 borderColor = ImVec4(0.20f, 0.19f, 0.17f, 0.45f); // Строгий контур
        ImVec4 textColor = ImVec4(0.20f, 0.19f, 0.17f, 0.90f); // Угольный текст

        // 1. Строгая прямоугольная плашка без скруглений
        drawList->AddRectFilled(letterMin, letterMax, ImGui::ColorConvertFloat4ToU32(panelBg), 0.0f);
        drawList->AddRect(letterMin, letterMax, ImGui::ColorConvertFloat4ToU32(borderColor), 0.0f, 0, 1.5f);

        // Левая акцентная полоса (характерная для интерфейсов NieR)
        drawList->AddRectFilled(
            ImVec2(letterMin.x, letterMin.y),
            ImVec2(letterMin.x + 5.0f, letterMax.y),
            ImGui::ColorConvertFloat4ToU32(borderColor)
        );

        // 2. Контент внутри карточки
        ImGui::SetCursorPos(ImVec2(letterX + 25.0f, letterY + 18.0f));
        ImGui::BeginGroup();

        // Заголовок в строгих скобках вместо сломанной иконки ✉
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.20f, 0.19f, 0.17f, 1.0f));
        ImGui::Text(Localization::get("letter").c_str());
        ImGui::PopStyleColor();

        // Разделительная линия
        ImVec2 lineStart = ImGui::GetCursorScreenPos();
        lineStart.y += 6.0f;
        ImVec2 lineEnd = ImVec2(lineStart.x + letterWidth - 50.0f, lineStart.y);
        drawList->AddLine(lineStart, lineEnd, ImGui::ColorConvertFloat4ToU32(borderColor), 1.5f);

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15.0f);

        // 3. Область с текстом
        ImGui::BeginChild("CommanderLetterScroll", ImVec2(letterWidth - 45.0f, letterHeight - 75.0f), false, ImGuiWindowFlags_NoBackground);

        ImGui::PushStyleColor(ImGuiCol_Text, textColor);
        ImGui::Text(Localization::get("4.1").c_str());
        ImGui::Text(Localization::get("5.1").c_str());
        ImGui::Text(Localization::get("6.1").c_str());
        ImGui::Text(Localization::get("7.1").c_str());
        ImGui::Text(Localization::get("8.1").c_str());
        ImGui::Text(Localization::get("9.1").c_str());
        ImGui::Text(Localization::get("10.1").c_str());
        ImGui::Text(Localization::get("11.1").c_str());
        ImGui::Text(Localization::get("12.1").c_str());
        ImGui::Text(Localization::get("13.1").c_str());
        ImGui::PopStyleColor();

        ImGui::EndChild();
        ImGui::EndGroup();
    }
    // ==========================================
    // 3. ПРАВАЯ ЧАСТЬ: ОПЦИИ ДЛЯ ВКЛАДКИ "Язык"
    // ==========================================
    if (currentSubMenu == 1 && activeSettingTab == 0) {
        // Смещаем позицию вывода вправо от левой колонки категорий
        ImGui::SetCursorPos(ImVec2(lineStartX - windowPos.x + currentButtonWidth + 80.0f, buttonsStartY));
        ImGui::BeginGroup();

        // Стилизуем сам выпадающий список ImGui, чтобы он подходил по цвету под NieR
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));         // Полностью прозрачная подложка комбобокса
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));  // Убираем реакцию на наведение у стандартного комбобокса
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.90f, 0.89f, 0.83f, 1.0f));       // Светло-бежевый фон выпадающего списка
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.19f, 0.17f, 0.25f));        // Цвет выделения элемента в списке
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.19f, 0.17f, 0.4f));

        // Умный виджет настройки: внешне NieR, по клику — красивый выпадающий список
        auto DrawNieRDropdownSetting = [&](const char* labelKey, int* currentIndex, const char* options[], int optionsCount) {
            ImVec2 screenPos = ImGui::GetCursorScreenPos();
            float settingWidth = 550.0f; // Длина прямоугольной плашки

            std::string label = Localization::get(labelKey);
            if (label.empty()) label = labelKey;

            const char* valueText = options[*currentIndex];

            // 1. Делаем невидимую кнопку-подложку на ВЕСЬ размер прямоугольника
            std::string btnId = std::string("##btn_") + labelKey;
            ImGui::SetNextItemWidth(settingWidth);

            // Заставляем ImGui думать, что кнопка прозрачная, но обрабатывает клики
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
            bool pressed = ImGui::Button(btnId.c_str(), ImVec2(settingWidth, bH));
            ImGui::PopStyleColor(3);

            bool isHovered = ImGui::IsItemHovered();
            ImDrawList* wDrawList = ImGui::GetWindowDrawList();

            // 2. Отрисовка фонового прямоугольника NieR
            ImVec4 bgColor = isHovered ? ImVec4(0.2f, 0.19f, 0.17f, 0.28f) : ImVec4(0.2f, 0.19f, 0.17f, 0.12f);
            wDrawList->AddRectFilled(screenPos, ImVec2(screenPos.x + settingWidth, screenPos.y + bH), ImGui::ColorConvertFloat4ToU32(bgColor));

            // Цвет элементов интерфейса
            ImU32 elementColor = isHovered
                ? ImGui::ColorConvertFloat4ToU32(ImVec4(0.20f, 0.19f, 0.17f, 1.0f))
                : ImGui::ColorConvertFloat4ToU32(ImVec4(0.20f, 0.19f, 0.17f, 0.75f));

            // 3. Рисуем квадратный маркер слева
            float squareSize = 14.0f;
            float sqX = screenPos.x + 18.0f;
            float sqY = screenPos.y + (bH - squareSize) * 0.5f;
            wDrawList->AddRectFilled(ImVec2(sqX, sqY), ImVec2(sqX + squareSize, sqY + squareSize), elementColor);

            // 4. Рисуем название опции слева
            ImVec2 textPos = ImVec2(screenPos.x + 45.0f, screenPos.y + (bH - ImGui::GetTextLineHeight()) * 0.5f);
            wDrawList->AddText(textPos, elementColor, label.c_str());

            // 5. Рисуем текущее значение опции справа
            ImVec2 valSize = ImGui::CalcTextSize(valueText);
            ImVec2 valPos = ImVec2(screenPos.x + settingWidth - valSize.x - 25.0f, screenPos.y + (bH - valSize.y) * 0.5f);
            wDrawList->AddText(valPos, elementColor, valueText);

            // ИД для нашего кастомного поп-апа
            std::string popupId = std::string("popup_") + labelKey;

            // Если кликнули на прямоугольник — открываем меню
            if (pressed) {
                ImGui::OpenPopup(popupId.c_str());
            }

            // Позиционируем выпадающий список ровно под плашкой
            ImGui::SetNextWindowPos(ImVec2(screenPos.x, screenPos.y + bH));
            ImGui::SetNextWindowSize(ImVec2(settingWidth, 0.0f)); // Ширина как у плашки

            // Стилизуем окно выпадающего списка под NieR
            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.90f, 0.89f, 0.83f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.19f, 0.17f, 0.25f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.19f, 0.17f, 0.4f));

            if (ImGui::BeginPopup(popupId.c_str())) {
                for (int i = 0; i < optionsCount; i++) {
                    bool isSelected = (*currentIndex == i);
                    if (ImGui::Selectable(options[i], isSelected)) {
                        *currentIndex = i;
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndPopup();
            }
            ImGui::PopStyleColor(3);
            };

        // 1. Язык
        std::string labelRu = Localization::get("russian"); if (labelRu.empty()) labelRu = "Русский";
        const char* language[] = { labelRu.c_str(), "English",};
        DrawNieRDropdownSetting("language", &renderData.rdLanguageIndex, language, IM_ARRAYSIZE(language));
        ImGui::Dummy(ImVec2(0, 10.0f));

      

        
        ImGui::PopStyleColor(6);
        ImGui::EndGroup();
    }
    // ==========================================
    // 4. ПРАВАЯ ЧАСТЬ: ОПЦИИ ДЛЯ ВКЛАДКИ "ГРАФИКА"
    // ==========================================
    if (currentSubMenu == 1 && activeSettingTab == 2) {
        // Смещаем позицию вывода вправо от левой колонки категорий
        ImGui::SetCursorPos(ImVec2(lineStartX - windowPos.x + currentButtonWidth + 80.0f, buttonsStartY));
        ImGui::BeginGroup();

        // Стилизуем сам выпадающий список ImGui, чтобы он подходил по цвету под NieR
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));         // Полностью прозрачная подложка комбобокса
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));  // Убираем реакцию на наведение у стандартного комбобокса
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.90f, 0.89f, 0.83f, 1.0f));       // Светло-бежевый фон выпадающего списка
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.19f, 0.17f, 0.25f));        // Цвет выделения элемента в списке
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.19f, 0.17f, 0.4f));

        // Умный виджет настройки: внешне NieR, по клику — красивый выпадающий список
        auto DrawNieRDropdownSetting = [&](const char* labelKey, int* currentIndex, const char* options[], int optionsCount) {
            ImVec2 screenPos = ImGui::GetCursorScreenPos();
            float settingWidth = 550.0f; // Длина прямоугольной плашки

            std::string label = Localization::get(labelKey);
            if (label.empty()) label = labelKey;

            const char* valueText = options[*currentIndex];

            // 1. Делаем невидимую кнопку-подложку на ВЕСЬ размер прямоугольника
            std::string btnId = std::string("##btn_") + labelKey;
            ImGui::SetNextItemWidth(settingWidth);

            // Заставляем ImGui думать, что кнопка прозрачная, но обрабатывает клики
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
            bool pressed = ImGui::Button(btnId.c_str(), ImVec2(settingWidth, bH));
            ImGui::PopStyleColor(3);

            bool isHovered = ImGui::IsItemHovered();
            ImDrawList* wDrawList = ImGui::GetWindowDrawList();

            // 2. Отрисовка фонового прямоугольника NieR
            ImVec4 bgColor = isHovered ? ImVec4(0.2f, 0.19f, 0.17f, 0.28f) : ImVec4(0.2f, 0.19f, 0.17f, 0.12f);
            wDrawList->AddRectFilled(screenPos, ImVec2(screenPos.x + settingWidth, screenPos.y + bH), ImGui::ColorConvertFloat4ToU32(bgColor));

            // Цвет элементов интерфейса
            ImU32 elementColor = isHovered
                ? ImGui::ColorConvertFloat4ToU32(ImVec4(0.20f, 0.19f, 0.17f, 1.0f))
                : ImGui::ColorConvertFloat4ToU32(ImVec4(0.20f, 0.19f, 0.17f, 0.75f));

            // 3. Рисуем квадратный маркер слева
            float squareSize = 14.0f;
            float sqX = screenPos.x + 18.0f;
            float sqY = screenPos.y + (bH - squareSize) * 0.5f;
            wDrawList->AddRectFilled(ImVec2(sqX, sqY), ImVec2(sqX + squareSize, sqY + squareSize), elementColor);

            // 4. Рисуем название опции слева
            ImVec2 textPos = ImVec2(screenPos.x + 45.0f, screenPos.y + (bH - ImGui::GetTextLineHeight()) * 0.5f);
            wDrawList->AddText(textPos, elementColor, label.c_str());

            // 5. Рисуем текущее значение опции справа
            ImVec2 valSize = ImGui::CalcTextSize(valueText);
            ImVec2 valPos = ImVec2(screenPos.x + settingWidth - valSize.x - 25.0f, screenPos.y + (bH - valSize.y) * 0.5f);
            wDrawList->AddText(valPos, elementColor, valueText);

            // ИД для нашего кастомного поп-апа
            std::string popupId = std::string("popup_") + labelKey;

            // Если кликнули на прямоугольник — открываем меню
            if (pressed) {
                ImGui::OpenPopup(popupId.c_str());
            }

            // Позиционируем выпадающий список ровно под плашкой
            ImGui::SetNextWindowPos(ImVec2(screenPos.x, screenPos.y + bH));
            ImGui::SetNextWindowSize(ImVec2(settingWidth, 0.0f)); // Ширина как у плашки

            // Стилизуем окно выпадающего списка под NieR
            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.90f, 0.89f, 0.83f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.19f, 0.17f, 0.25f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.19f, 0.17f, 0.4f));

            if (ImGui::BeginPopup(popupId.c_str())) {
                for (int i = 0; i < optionsCount; i++) {
                    bool isSelected = (*currentIndex == i);
                    if (ImGui::Selectable(options[i], isSelected)) {
                        *currentIndex = i;
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndPopup();
            }
            ImGui::PopStyleColor(3);
            };

        // 1. Разрешение экрана
      
        const char* resolutions[] = { "1280x720", "1920x1080", "2560x1440", "3840x2160" };
        DrawNieRDropdownSetting("resolution", &renderData.rdResolutionIndex, resolutions, IM_ARRAYSIZE(resolutions));
        ImGui::Dummy(ImVec2(0, 10.0f));

        // Кешируем переводы статусов "Вкл / Выкл"
        std::string labelOff = Localization::get("off"); if (labelOff.empty()) labelOff = "Выкл.";
        std::string labelOn = Localization::get("on");   if (labelOn.empty()) labelOn = "Вкл.";
        const char* toggleOptions[] = { labelOff.c_str(), labelOn.c_str() };

        // 2. Полноэкранный режим
        
        DrawNieRDropdownSetting("fullscreen", &renderData.rdFullscreenIndex, toggleOptions, IM_ARRAYSIZE(toggleOptions));
        ImGui::Dummy(ImVec2(0, 10.0f));

        // 3. Вертикальная синхронизация
       
        DrawNieRDropdownSetting("vsync", &renderData.rdVSyncIndex, toggleOptions, IM_ARRAYSIZE(toggleOptions));
        ImGui::Dummy(ImVec2(0, 10.0f));

        // 4. Сглаживание
        
        const char* msaaOptions[] = { labelOff.c_str(), "MSAA 2x", "MSAA 4x", "MSAA 8x" };
        DrawNieRDropdownSetting("antialiasing", &renderData.rdMSAAIndex, msaaOptions, IM_ARRAYSIZE(msaaOptions));
        ImGui::Dummy(ImVec2(0, 10.0f));

        // 5. Фильтрация текстур
        
        const char* afOptions[] = { labelOff.c_str(), "Anisotropic 2x", "Anisotropic 4x", "Anisotropic 8x", "Anisotropic 16x" };
        DrawNieRDropdownSetting("texture_filtering", &renderData.rdAnisotropyIndex, afOptions, IM_ARRAYSIZE(afOptions));

        // 6. Качество текстур (Mipmapping)
        // 6. Качество текстур (Mipmapping)
        std::string labelUltra = Localization::get("ultra");  if (labelUltra.empty())  labelUltra = "Ультра (Макс.)";
        std::string labelHigh = Localization::get("high");   if (labelHigh.empty())   labelHigh = "Высокое";
        std::string labelMedium = Localization::get("medium"); if (labelMedium.empty()) labelMedium = "Среднее";
        std::string labelLow = Localization::get("low");    if (labelLow.empty())    labelLow = "Низкое (Картошка)";

        const char* qualities[] = {
            labelUltra.c_str(),
            labelHigh.c_str(),
            labelMedium.c_str(),
            labelLow.c_str()
        };
        DrawNieRDropdownSetting("texture_quality", &renderData.rdTextureQuality, qualities, IM_ARRAYSIZE(qualities));

        ImGui::PopStyleColor(6);
        ImGui::EndGroup();
    }

    // ==========================================
    // ЛИНИЯ И КНОПКА "НАЗАД" СТРОГО ВНИЗУ ОКНА
    // ==========================================
    if (currentSubMenu == 1) {
        ImVec2 windowSize = ImGui::GetWindowSize();

        // Позиция линии: ровно в 120 пикселях от низа экрана
        float footerLineY = windowPos.y + windowSize.y - 120.0f;

        // Рисуем сплошную тонкую линию NieR на всю ширину окна поверх всего
        ImU32 footerLineColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.20f, 0.19f, 0.17f, 0.4f));
        drawList->AddLine(ImVec2(windowPos.x, footerLineY), ImVec2(windowPos.x + windowSize.x, footerLineY), footerLineColor, 1.5f);

        // Сдвигаем курсор под линию и выравниваем по иксу с левыми кнопками категории
        ImGui::SetCursorScreenPos(ImVec2(lineStartX + 35.0f, footerLineY + 25.0f));

        // Вызываем уже готовую лямбду кнопки "Назад"
        if (DrawNieRButton("back")) {
            currentSubMenu = 0;
        }
    }

    ImGui::End(); // Закрываем SYSTEM_SCREEN
}


void UserInterface::createFrame(OGLRenderData &renderData, ModelSettings &settings, bool mShowMenu, bool debug) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  if (debug == true) {
      if (ImGui::IsKeyPressed(ImGuiKey_F1)) {
          renderData.mIsDeveloperMode = !renderData.mIsDeveloperMode;
      }
      drawSystemMenu(renderData, mShowMenu);
      if (renderData.mIsDeveloperMode) {
          // Твое текущее окно "Control" со всеми ползунками
          drawDeveloperUI(renderData, settings);
      }
      else {
          // Тот самый минималистичный NieR HUD
          drawPlayerHUD(renderData, settings);
      }
  }
}

void UserInterface::render() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UserInterface::cleanup() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}
