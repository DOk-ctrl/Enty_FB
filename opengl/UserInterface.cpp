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
        ImGuiStyle* style = &ImGui::GetStyle();         {
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
    ImGuiIO& io = ImGui::GetIO();

    ImFontConfig font_config;
  font_config.OversampleH = 1;
  font_config.OversampleV = 1;
  font_config.PixelSnapH = true;

    static const ImWchar cyrillic_ranges[] = {
      0x0020, 0x00FF,       0x0400, 0x044F,       0,
  };

 
  io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/Arial.ttf", 16.0f, &font_config, cyrillic_ranges);
 
    io.Fonts->AddFontFromFileTTF("assets/fonts/Rajdhani/Rajdhani-Bold.ttf", 120.0f);
  io.Fonts->AddFontFromFileTTF("assets/fonts/Monomaniac_One/MonomaniacOne-Regular.ttf", 36.0f);
    io.Fonts->AddFontFromFileTTF("assets/fonts/ExtraBold.ttf", 22.0f, &font_config, cyrillic_ranges);
    style();
  /* init plot vectors */
    
  mFPSValues.resize(mNumFPSValues);
  mFrameTimeValues.resize(mNumFrameTimeValues);
  mModelUploadValues.resize(mNumModelUploadValues);
  mMatrixGenerationValues.resize(mNumMatrixGenerationValues);
  mIKValues.resize(mNumIKValues);
  mMatrixUploadValues.resize(mNumMatrixUploadValues);
  mUiGenValues.resize(mNumUiGenValues);
  mUiDrawValues.resize(mNumUiDrawValues);
  
}
void UserInterface::drawDeveloperUI(OGLRenderData& renderData, ModelSettings& settings) {   
    
}
void UserInterface::drawPlayerHUD(OGLRenderData& renderData, ModelSettings& settings) {
    ImVec2 screenSize = ImGui::GetIO().DisplaySize;

    ImVec4 nierColorText = ImVec4(0.72f, 0.70f, 0.60f, 0.9f);
    ImU32 nierColorBar = IM_COL32(160, 155, 130, 255);

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

                float questWidth = 340.0f;
    float rightPadding = 40.0f;
    float topPadding = 45.0f;

    ImGui::SetNextWindowPos(ImVec2(screenSize.x - rightPadding, topPadding), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(questWidth, 0.0f));

    if (ImGui::Begin("QuestHUD", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();

                ImVec4 headerTextColor;
        ImU32 headerLineColor;

        if (renderData.rdIsGameWon) {
                        headerTextColor = ImVec4(0.25f, 0.88f, 0.42f, 1.0f);
            headerLineColor = IM_COL32(50, 210, 100, 230);
        }
        else {
                        headerTextColor = ImVec4(0.98f, 0.80f, 0.25f, 1.0f);
            headerLineColor = IM_COL32(240, 185, 45, 220);
        }

        ImU32 shadowLineColor = IM_COL32(10, 10, 10, 180);

                ImGui::PushStyleColor(ImGuiCol_Text, headerTextColor);
        if (renderData.rdIsGameWon) {
            ImGui::Text(Localization::get("goalT").c_str());
        }
        else {
            ImGui::Text(Localization::get("goal").c_str());
        }
        ImGui::PopStyleColor();

                ImVec2 lineStart = ImGui::GetCursorScreenPos();
        lineStart.y += 4.0f;
        ImVec2 lineEnd = ImVec2(lineStart.x + questWidth, lineStart.y);

        drawList->AddLine(ImVec2(lineStart.x, lineStart.y + 1.5f), ImVec2(lineEnd.x, lineEnd.y + 1.5f), shadowLineColor, 2.0f);
        drawList->AddLine(lineStart, lineEnd, headerLineColor, 2.0f);

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);

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

    static int activeSettingTab = 2; 
    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiStyle oldStyle = style;

        style.WindowRounding = 14.0f;
    style.FrameRounding = 10.0f;
    style.PopupRounding = 10.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;

        ImVec4 bgDark = ImVec4(0.08f, 0.09f, 0.18f, 0.88f);     ImVec4 panelBg = ImVec4(0.18f, 0.18f, 0.32f, 0.45f);     ImVec4 buttonNormal = ImVec4(0.25f, 0.24f, 0.42f, 0.70f);     ImVec4 buttonHovered = ImVec4(0.38f, 0.35f, 0.65f, 0.90f);     ImVec4 buttonActive = ImVec4(0.18f, 0.55f, 0.95f, 0.90f);     ImVec4 textColor = ImVec4(0.92f, 0.95f, 1.00f, 0.95f);     ImVec4 accentCyan = ImVec4(0.20f, 0.75f, 1.00f, 1.00f); 
    style.Colors[ImGuiCol_WindowBg] = bgDark;
    style.Colors[ImGuiCol_Border] = ImVec4(0.35f, 0.45f, 0.80f, 0.30f);
    style.Colors[ImGuiCol_Text] = textColor;
    style.Colors[ImGuiCol_Button] = buttonNormal;
    style.Colors[ImGuiCol_ButtonHovered] = buttonHovered;
    style.Colors[ImGuiCol_ButtonActive] = buttonActive;

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

        std::string rawTitle = Localization::get("settings");
    if (rawTitle.empty()) rawTitle = "НАСТРОЙКИ";

    ImGui::SetCursorPos(ImVec2(35.0f, 25.0f));
    ImGui::SetWindowFontScale(1.4f);
    ImGui::TextColored(accentCyan, "%s", rawTitle.c_str());
    ImGui::SetWindowFontScale(1.0f);

        float lineY = winPos.y + 70.0f;
    drawList->AddLine(
        ImVec2(winPos.x + 35.0f, lineY),
        ImVec2(winPos.x + winSize.x - 35.0f, lineY),
        ImGui::ColorConvertFloat4ToU32(ImVec4(0.35f, 0.55f, 0.95f, 0.40f)),
        2.0f
    );

        float contentStartY = 95.0f;
    float leftColWidth = 240.0f;
    float itemHeight = 48.0f;

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

        ImGui::SetCursorPos(ImVec2(35.0f, contentStartY));
    ImGui::BeginGroup();

    DrawCategoryTab("language", 0); ImGui::Dummy(ImVec2(0, 8.0f));
    DrawCategoryTab("camera", 1);   ImGui::Dummy(ImVec2(0, 8.0f));
    DrawCategoryTab("grathics", 2); ImGui::Dummy(ImVec2(0, 8.0f));

    ImGui::EndGroup();

        float sepX = winPos.x + 35.0f + leftColWidth + 25.0f;
    drawList->AddLine(
        ImVec2(sepX, winPos.y + contentStartY),
        ImVec2(sepX, winPos.y + winSize.y - 90.0f),
        ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.12f)),
        1.5f
    );

        float rightColX = leftColWidth + 85.0f;
    float dropdownWidth = 480.0f;

        auto DrawOptionDropdown = [&](const char* labelKey, int* currentIndex, const char* options[], int optionsCount) {
        ImVec2 screenPos = ImGui::GetCursorScreenPos();
        std::string label = Localization::get(labelKey);
        if (label.empty()) label = labelKey;

        const char* currentValText = options[*currentIndex];

                drawList->AddRectFilled(
            screenPos,
            ImVec2(screenPos.x + dropdownWidth, screenPos.y + itemHeight),
            ImGui::ColorConvertFloat4ToU32(panelBg),
            10.0f
        );

                ImVec2 textPos = ImVec2(screenPos.x + 20.0f, screenPos.y + (itemHeight - ImGui::GetTextLineHeight()) * 0.5f);
        drawList->AddText(textPos, ImGui::ColorConvertFloat4ToU32(textColor), label.c_str());

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

                ImGui::SetCursorScreenPos(ImVec2(screenPos.x, screenPos.y + itemHeight + 10.0f));
        };

        if (activeSettingTab == 0) {
        ImGui::SetCursorPos(ImVec2(rightColX, contentStartY));
        ImGui::BeginGroup();

        std::string labelRu = Localization::get("russian"); if (labelRu.empty()) labelRu = "Русский";
        const char* language[] = { labelRu.c_str(), "English" };
        DrawOptionDropdown("language", &renderData.rdLanguageIndex, language, IM_ARRAYSIZE(language));

        ImGui::EndGroup();
    }

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
        DrawOptionDropdown("Enterprise anime shader", &renderData.rdEntyAnim, toggleOptions, IM_ARRAYSIZE(toggleOptions));
        ImGui::EndGroup();
    }
    
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

        style = oldStyle;
}

void UserInterface::drawSystemMenu(OGLRenderData& renderData, bool& showMenu) {
    if (!showMenu) return;

        static int currentSubMenu = 0;       static int activeSettingTab = 2; 
    ImGuiStyle& style = ImGui::GetStyle();

        ImVec4 oldWinBg = style.Colors[ImGuiCol_WindowBg];
    ImVec4 oldText = style.Colors[ImGuiCol_Text];

        style.WindowRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.WindowBorderSize = 0.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.95f, 0.94f, 0.886f, 1.0f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.20f, 0.19f, 0.17f, 0.90f);

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)renderData.rdWidth, (float)renderData.rdHeight));

    ImGui::Begin("SYSTEM_SCREEN", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

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

        style.Colors[ImGuiCol_Text] = ImVec4(0.20f, 0.19f, 0.17f, 0.90f);

    ImGui::SetCursorPosY(topMargin + tabHeight + 10.0f);
    ImGui::Separator();
    ImGui::SetCursorPosY(topMargin + tabHeight + 13.0f);
    ImGui::Separator();

                static float testWidth = 615.926f;
    static float testHeight = 78.424f;
    static float testSpacing = 34.66f;

    static float lineThickness = 9.333f;
    static float thinLineOffset = 12.582f;
    static float thinLineThickness = 2.609f;
    static float lineLengthFactor = 0.65f;

    float leftMargin = renderData.rdWidth * 0.08f;
    float buttonsStartY = renderData.rdHeight * 0.24f;

                float titleX = leftMargin;
    float titleY = buttonsStartY - 110.0f;

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

                ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();

    float lineStartX = windowPos.x + 130.0f;
    float lineStartY = windowPos.y + buttonsStartY - 10.0f;
    float lineEndY = windowPos.y + renderData.rdHeight * lineLengthFactor;

    ImU32 lineColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.20f, 0.19f, 0.17f, 0.4f));

    drawList->AddLine(ImVec2(lineStartX, lineStartY), ImVec2(lineStartX, lineEndY), lineColor, lineThickness);
    float thinLineX = lineStartX + thinLineOffset;
    drawList->AddLine(ImVec2(thinLineX, lineStartY), ImVec2(thinLineX, lineEndY), lineColor, thinLineThickness);

                ImGui::SetCursorPos(ImVec2(lineStartX - windowPos.x + 35.0f, buttonsStartY));
    ImGui::BeginGroup();

    float bW = testWidth;
    float bH = testHeight;

        float currentButtonWidth = (currentSubMenu == 1) ? 260.0f : bW;

        auto DrawNieRButton = [&](const char* localizationKey, bool isActiveTab = false) {
        ImVec2 screenPos = ImGui::GetCursorScreenPos();

                std::string label = Localization::get(localizationKey);

                if (label.empty()) {
            label = localizationKey;
        }

                if (isActiveTab) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.19f, 0.17f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.19f, 0.17f, 0.8f));
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.19f, 0.17f, 0.1f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.19f, 0.17f, 0.8f));
        }
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.19f, 0.17f, 0.9f));

                if (isActiveTab) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.94f, 0.886f, 1.0f));
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.20f, 0.19f, 0.17f, 0.90f));
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(45.0f, style.FramePadding.y));

                bool pressed = ImGui::Button(label.c_str(), ImVec2(currentButtonWidth, bH));
        bool isHovered = ImGui::IsItemHovered() || isActiveTab;

        ImDrawList* wDrawList = ImGui::GetWindowDrawList();

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

        if (currentSubMenu == 0) {
                if (DrawNieRButton("save")) { /* код сохранения */ } ImGui::Dummy(ImVec2(0, testSpacing));
        if (DrawNieRButton("load")) { /* код загрузки */ }   ImGui::Dummy(ImVec2(0, testSpacing));
        if (DrawNieRButton("settings")) { currentSubMenu = 1; }  ImGui::Dummy(ImVec2(0, testSpacing));
        if (DrawNieRButton("exit")) { glfwSetWindowShouldClose(renderData.rdWindow, true); }
    }
    else if (currentSubMenu == 1) {
                if (DrawNieRButton("language", activeSettingTab == 0)) { activeSettingTab = 0; } ImGui::Dummy(ImVec2(0, 12.0f));
        if (DrawNieRButton("camera", activeSettingTab == 1)) { activeSettingTab = 1; } ImGui::Dummy(ImVec2(0, 12.0f));
        if (DrawNieRButton("grathics", activeSettingTab == 2)) { activeSettingTab = 2; } ImGui::Dummy(ImVec2(0, 12.0f));

            }

    ImGui::EndGroup();

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

                ImVec4 panelBg = ImVec4(0.20f, 0.19f, 0.17f, 0.05f);         ImVec4 borderColor = ImVec4(0.20f, 0.19f, 0.17f, 0.45f);         ImVec4 textColor = ImVec4(0.20f, 0.19f, 0.17f, 0.90f); 
                drawList->AddRectFilled(letterMin, letterMax, ImGui::ColorConvertFloat4ToU32(panelBg), 0.0f);
        drawList->AddRect(letterMin, letterMax, ImGui::ColorConvertFloat4ToU32(borderColor), 0.0f, 0, 1.5f);

                drawList->AddRectFilled(
            ImVec2(letterMin.x, letterMin.y),
            ImVec2(letterMin.x + 5.0f, letterMax.y),
            ImGui::ColorConvertFloat4ToU32(borderColor)
        );

                ImGui::SetCursorPos(ImVec2(letterX + 25.0f, letterY + 18.0f));
        ImGui::BeginGroup();

                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.20f, 0.19f, 0.17f, 1.0f));
        ImGui::Text(Localization::get("letter").c_str());
        ImGui::PopStyleColor();

                ImVec2 lineStart = ImGui::GetCursorScreenPos();
        lineStart.y += 6.0f;
        ImVec2 lineEnd = ImVec2(lineStart.x + letterWidth - 50.0f, lineStart.y);
        drawList->AddLine(lineStart, lineEnd, ImGui::ColorConvertFloat4ToU32(borderColor), 1.5f);

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15.0f);

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
                if (currentSubMenu == 1 && activeSettingTab == 0) {
                ImGui::SetCursorPos(ImVec2(lineStartX - windowPos.x + currentButtonWidth + 80.0f, buttonsStartY));
        ImGui::BeginGroup();

                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));                 ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));          ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.90f, 0.89f, 0.83f, 1.0f));               ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.19f, 0.17f, 0.25f));                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.19f, 0.17f, 0.4f));

                auto DrawNieRDropdownSetting = [&](const char* labelKey, int* currentIndex, const char* options[], int optionsCount) {
            ImVec2 screenPos = ImGui::GetCursorScreenPos();
            float settingWidth = 550.0f; 
            std::string label = Localization::get(labelKey);
            if (label.empty()) label = labelKey;

            const char* valueText = options[*currentIndex];

                        std::string btnId = std::string("##btn_") + labelKey;
            ImGui::SetNextItemWidth(settingWidth);

                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
            bool pressed = ImGui::Button(btnId.c_str(), ImVec2(settingWidth, bH));
            ImGui::PopStyleColor(3);

            bool isHovered = ImGui::IsItemHovered();
            ImDrawList* wDrawList = ImGui::GetWindowDrawList();

                        ImVec4 bgColor = isHovered ? ImVec4(0.2f, 0.19f, 0.17f, 0.28f) : ImVec4(0.2f, 0.19f, 0.17f, 0.12f);
            wDrawList->AddRectFilled(screenPos, ImVec2(screenPos.x + settingWidth, screenPos.y + bH), ImGui::ColorConvertFloat4ToU32(bgColor));

                        ImU32 elementColor = isHovered
                ? ImGui::ColorConvertFloat4ToU32(ImVec4(0.20f, 0.19f, 0.17f, 1.0f))
                : ImGui::ColorConvertFloat4ToU32(ImVec4(0.20f, 0.19f, 0.17f, 0.75f));

                        float squareSize = 14.0f;
            float sqX = screenPos.x + 18.0f;
            float sqY = screenPos.y + (bH - squareSize) * 0.5f;
            wDrawList->AddRectFilled(ImVec2(sqX, sqY), ImVec2(sqX + squareSize, sqY + squareSize), elementColor);

                        ImVec2 textPos = ImVec2(screenPos.x + 45.0f, screenPos.y + (bH - ImGui::GetTextLineHeight()) * 0.5f);
            wDrawList->AddText(textPos, elementColor, label.c_str());

                        ImVec2 valSize = ImGui::CalcTextSize(valueText);
            ImVec2 valPos = ImVec2(screenPos.x + settingWidth - valSize.x - 25.0f, screenPos.y + (bH - valSize.y) * 0.5f);
            wDrawList->AddText(valPos, elementColor, valueText);

                        std::string popupId = std::string("popup_") + labelKey;

                        if (pressed) {
                ImGui::OpenPopup(popupId.c_str());
            }

                        ImGui::SetNextWindowPos(ImVec2(screenPos.x, screenPos.y + bH));
            ImGui::SetNextWindowSize(ImVec2(settingWidth, 0.0f)); 
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

                std::string labelRu = Localization::get("russian"); if (labelRu.empty()) labelRu = "Русский";
        const char* language[] = { labelRu.c_str(), "English",};
        DrawNieRDropdownSetting("language", &renderData.rdLanguageIndex, language, IM_ARRAYSIZE(language));
        ImGui::Dummy(ImVec2(0, 10.0f));

      

        
        ImGui::PopStyleColor(6);
        ImGui::EndGroup();
    }
                if (currentSubMenu == 1 && activeSettingTab == 2) {
                ImGui::SetCursorPos(ImVec2(lineStartX - windowPos.x + currentButtonWidth + 80.0f, buttonsStartY));
        ImGui::BeginGroup();

                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));                 ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));          ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.90f, 0.89f, 0.83f, 1.0f));               ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.19f, 0.17f, 0.25f));                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.19f, 0.17f, 0.4f));

                auto DrawNieRDropdownSetting = [&](const char* labelKey, int* currentIndex, const char* options[], int optionsCount) {
            ImVec2 screenPos = ImGui::GetCursorScreenPos();
            float settingWidth = 550.0f; 
            std::string label = Localization::get(labelKey);
            if (label.empty()) label = labelKey;

            const char* valueText = options[*currentIndex];

                        std::string btnId = std::string("##btn_") + labelKey;
            ImGui::SetNextItemWidth(settingWidth);

                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
            bool pressed = ImGui::Button(btnId.c_str(), ImVec2(settingWidth, bH));
            ImGui::PopStyleColor(3);

            bool isHovered = ImGui::IsItemHovered();
            ImDrawList* wDrawList = ImGui::GetWindowDrawList();

                        ImVec4 bgColor = isHovered ? ImVec4(0.2f, 0.19f, 0.17f, 0.28f) : ImVec4(0.2f, 0.19f, 0.17f, 0.12f);
            wDrawList->AddRectFilled(screenPos, ImVec2(screenPos.x + settingWidth, screenPos.y + bH), ImGui::ColorConvertFloat4ToU32(bgColor));

                        ImU32 elementColor = isHovered
                ? ImGui::ColorConvertFloat4ToU32(ImVec4(0.20f, 0.19f, 0.17f, 1.0f))
                : ImGui::ColorConvertFloat4ToU32(ImVec4(0.20f, 0.19f, 0.17f, 0.75f));

                        float squareSize = 14.0f;
            float sqX = screenPos.x + 18.0f;
            float sqY = screenPos.y + (bH - squareSize) * 0.5f;
            wDrawList->AddRectFilled(ImVec2(sqX, sqY), ImVec2(sqX + squareSize, sqY + squareSize), elementColor);

                        ImVec2 textPos = ImVec2(screenPos.x + 45.0f, screenPos.y + (bH - ImGui::GetTextLineHeight()) * 0.5f);
            wDrawList->AddText(textPos, elementColor, label.c_str());

                        ImVec2 valSize = ImGui::CalcTextSize(valueText);
            ImVec2 valPos = ImVec2(screenPos.x + settingWidth - valSize.x - 25.0f, screenPos.y + (bH - valSize.y) * 0.5f);
            wDrawList->AddText(valPos, elementColor, valueText);

                        std::string popupId = std::string("popup_") + labelKey;

                        if (pressed) {
                ImGui::OpenPopup(popupId.c_str());
            }

                        ImGui::SetNextWindowPos(ImVec2(screenPos.x, screenPos.y + bH));
            ImGui::SetNextWindowSize(ImVec2(settingWidth, 0.0f)); 
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

              
        const char* resolutions[] = { "1280x720", "1920x1080", "2560x1440", "3840x2160" };
        DrawNieRDropdownSetting("resolution", &renderData.rdResolutionIndex, resolutions, IM_ARRAYSIZE(resolutions));
        ImGui::Dummy(ImVec2(0, 10.0f));

                std::string labelOff = Localization::get("off"); if (labelOff.empty()) labelOff = "Выкл.";
        std::string labelOn = Localization::get("on");   if (labelOn.empty()) labelOn = "Вкл.";
        const char* toggleOptions[] = { labelOff.c_str(), labelOn.c_str() };

                
        DrawNieRDropdownSetting("fullscreen", &renderData.rdFullscreenIndex, toggleOptions, IM_ARRAYSIZE(toggleOptions));
        ImGui::Dummy(ImVec2(0, 10.0f));

               
        DrawNieRDropdownSetting("vsync", &renderData.rdVSyncIndex, toggleOptions, IM_ARRAYSIZE(toggleOptions));
        ImGui::Dummy(ImVec2(0, 10.0f));

                
        const char* msaaOptions[] = { labelOff.c_str(), "MSAA 2x", "MSAA 4x", "MSAA 8x" };
        DrawNieRDropdownSetting("antialiasing", &renderData.rdMSAAIndex, msaaOptions, IM_ARRAYSIZE(msaaOptions));
        ImGui::Dummy(ImVec2(0, 10.0f));

                
        const char* afOptions[] = { labelOff.c_str(), "Anisotropic 2x", "Anisotropic 4x", "Anisotropic 8x", "Anisotropic 16x" };
        DrawNieRDropdownSetting("texture_filtering", &renderData.rdAnisotropyIndex, afOptions, IM_ARRAYSIZE(afOptions));

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
        
        DrawNieRDropdownSetting("Enterprise anime shader", &renderData.rdEntyAnim, toggleOptions, IM_ARRAYSIZE(toggleOptions));
        ImGui::PopStyleColor(6);
        ImGui::EndGroup();
    }

                if (currentSubMenu == 1) {
        ImVec2 windowSize = ImGui::GetWindowSize();

                float footerLineY = windowPos.y + windowSize.y - 120.0f;

                ImU32 footerLineColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.20f, 0.19f, 0.17f, 0.4f));
        drawList->AddLine(ImVec2(windowPos.x, footerLineY), ImVec2(windowPos.x + windowSize.x, footerLineY), footerLineColor, 1.5f);

                ImGui::SetCursorScreenPos(ImVec2(lineStartX + 35.0f, footerLineY + 25.0f));

                if (DrawNieRButton("back")) {
            currentSubMenu = 0;
        }
    }

    ImGui::End(); }


void UserInterface::createFrame(OGLRenderData &renderData, ModelSettings &settings, bool mShowMenu, bool debug) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  if (debug == true) {
      /*if (ImGui::IsKeyPressed(ImGuiKey_F1)) {
          renderData.mIsDeveloperMode = !renderData.mIsDeveloperMode;
      }
      */
      drawSystemMenu(renderData, mShowMenu);
      if (renderData.mIsDeveloperMode) {
          drawDeveloperUI(renderData, settings);
      }
      else {
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
