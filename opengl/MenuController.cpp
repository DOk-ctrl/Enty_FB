#define IMGUI_DEFINE_MATH_OPERATORS 
#define GLM_ENABLE_EXPERIMENTAL

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>

#include "MenuController.h"
#include "OGLRenderData.h"
#include "GltfModel.h"
#include "GltfInstance.h"
#include "Logger.h"
#include <Localization.h>
#include <UserInterface.h>
#include <JigglePhysics.h>
#include "audio.h"

bool MenuController::init(std::shared_ptr<GltfModel> model) {
    if (!model) {
        Logger::log(1, "%s error: null GltfModel passed\n", __FUNCTION__);
        return false;
    }
    mNJModel = model;
    gAudio.playMusic("assets/audio/mainMenu.MP3", true);    
    mNJInstance = std::make_shared<GltfInstance>(mNJModel, NJ_WORLD_POS, NJ_WORLD_ROT);
    {
        auto root = mNJInstance->getRootNode();

        auto leftNode = findNodeByName(root, "Breast.L");
        auto rightNode = findNodeByName(root, "Breast.R");

        if (leftNode || rightNode) {
            Logger::log(1, "%s  Breast.L/Breast.R found on Enty skeleton\n", __FUNCTION__);
        }
        if (!leftNode || !rightNode) {
            Logger::log(1, "%s warning: Breast.L/Breast.R not found on Enty skeleton\n", __FUNCTION__);
        }

        initBreastJiggle(mLeftBreastJiggleNJ, leftNode,20.0f,1.0f,40.0f,100.0f);
        initBreastJiggle(mRightBreastJiggleNJ, rightNode, 20.0f, 1.0f, 40.0f, 100.0f);
    }
    /* Получаем длительность Salute один раз при инициализации */
    mSaluteDuration = mNJInstance->getAnimationEndTime(ANIM_SALUTE);
    if (mSaluteDuration < 0.01f) {
        Logger::log(1, "%s warning: ANIM_SALUTE duration is %.3f — check clip index!\n",
            __FUNCTION__, mSaluteDuration);
        mSaluteDuration = 1.5f; /* fallback */
    }

    {
        auto s = mNJInstance->getInstanceSettings();
        s.msWorldPosition = NJ_WORLD_POS;
        s.msWorldRotation = NJ_WORLD_ROT;

        s.msAnimClip = ANIM_HIPHOP;
        s.msCrossBlendDestAnimClip = ANIM_HIPHOP;
        s.msBlendingMode = blendMode::crossfade;
        s.msAnimCrossBlendFactor = 1.0f;   
        s.msAnimTimePosition = 0.0f;
        s.msPlayAnimation = true;
        s.msAnimSpeed = 1.0f;

        mNJInstance->setInstanceSettings(s);
        mNJInstance->checkForUpdates();
    }

    mCurrentClip = ANIM_HIPHOP;
    mAnimState = MenuAnimState::HIPHOP;

    /* Логируем клипы чтобы пользователь мог проверить индексы */
    auto clips = mNJInstance->getInstanceSettings().msClipNames;
    Logger::log(1, "%s: NJ model loaded, animation clips:\n", __FUNCTION__);
    for (int i = 0; i < (int)clips.size(); ++i) {
        Logger::log(1, "  [%d] %s\n", i, clips[i].c_str());
    }

    return true;
}

/* ============================================================
 *  update()  — вызывается каждый кадр в MAIN_MENU состоянии
 * ============================================================ */
MenuAction MenuController::update(float deltaTime, bool anyButtonHovered) {
    /* Машина состояний анимации */
    updateBreastJiggle(mLeftBreastJiggleNJ, deltaTime);
    updateBreastJiggle(mRightBreastJiggleNJ, deltaTime);
    mNJInstance->refreshJointMatrices();
    switch (mAnimState) {

    case MenuAnimState::HIPHOP:
        /* Если навели на кнопку — начать кроссфейд в Idle */
        if (anyButtonHovered) {
            startCrossfade(ANIM_HIPHOP, ANIM_IDLE);
            mAnimState = MenuAnimState::CROSSFADE_IDLE;
        }
        break;

    case MenuAnimState::CROSSFADE_IDLE:
        tickCrossfade(deltaTime);
        /* Кроссфейд завершён */
        if (mBlendFactor >= 1.0f) {
            mCurrentClip = ANIM_IDLE;
            mAnimState = MenuAnimState::IDLE;
            /* Зафиксировать конечный клип как чистый loop */
            auto s = mNJInstance->getInstanceSettings();
            s.msAnimClip = ANIM_IDLE;
            s.msCrossBlendDestAnimClip = ANIM_IDLE;
            s.msAnimCrossBlendFactor = 1.0f;
            s.msPlayAnimation = true;
            mNJInstance->setInstanceSettings(s);
            mNJInstance->checkForUpdates();
        }
        /* Пока идёт кроссфейд курсор убрали — разворачиваемся обратно */
        if (!anyButtonHovered) {
            startCrossfade(ANIM_IDLE, ANIM_HIPHOP);
            mAnimState = MenuAnimState::CROSSFADE_HH;
        }
        break;

    case MenuAnimState::IDLE:
        if (!anyButtonHovered) {
            startCrossfade(ANIM_IDLE, ANIM_HIPHOP);
            mAnimState = MenuAnimState::CROSSFADE_HH;
        }
        break;

    case MenuAnimState::CROSSFADE_HH:
        tickCrossfade(deltaTime);
        if (mBlendFactor >= 1.0f) {
            mCurrentClip = ANIM_HIPHOP;
            mAnimState = MenuAnimState::HIPHOP;
            auto s = mNJInstance->getInstanceSettings();
            s.msAnimClip = ANIM_HIPHOP;
            s.msCrossBlendDestAnimClip = ANIM_HIPHOP;
            s.msAnimCrossBlendFactor = 1.0f;
            s.msPlayAnimation = true;
            mNJInstance->setInstanceSettings(s);
            mNJInstance->checkForUpdates();
        }
        if (anyButtonHovered) {
            startCrossfade(ANIM_HIPHOP, ANIM_IDLE);
            mAnimState = MenuAnimState::CROSSFADE_IDLE;
        }
        break;

    case MenuAnimState::SALUTE:
        tickSalute(deltaTime);
        /* Когда Salute кончился — сигнализируем в OGLRenderer */
        if (mSaluteFinished) {
            return MenuAction::START_GAME;
        }
        break;
    }

    /* Обновляем анимацию инстанса */
    mNJInstance->updateAnimation();
    return MenuAction::NONE;
}


bool CustomToggle(const char* label, bool* v, float width = 52.0f, float height = 26.0f) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

        ImGui::InvisibleButton(label, ImVec2(width, height));
    bool clicked = ImGui::IsItemClicked();
    bool hovered = ImGui::IsItemHovered();

    if (clicked) {
        *v = !(*v);
    }

        float t = *v ? 1.0f : 0.0f;
    float anim_speed = 12.0f;

        ImGuiID id = ImGui::GetID(label);
    float t_anim = ImGui::GetStateStorage()->GetFloat(id, t);

        float dt = ImGui::GetIO().DeltaTime;
    if (t_anim < t) t_anim = (t_anim + dt * anim_speed < t) ? t_anim + dt * anim_speed : t;
    else if (t_anim > t) t_anim = (t_anim - dt * anim_speed > t) ? t_anim - dt * anim_speed : t;

    ImGui::GetStateStorage()->SetFloat(id, t_anim);

        ImU32 bg_color = hovered
        ? ImColor(0.82f, 0.16f, 0.55f, 0.90f)           : (*v ? ImColor(0.65f, 0.15f, 0.50f, 0.85f)              : ImColor(0.28f, 0.08f, 0.22f, 0.70f)); 
    ImU32 knob_color = ImColor(0.98f, 0.94f, 0.98f, 1.00f);

    float radius = height * 0.5f;

        draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), bg_color, radius);

        float knob_pos_x = p.x + radius + t_anim * (width - radius * 2.0f);
    draw_list->AddCircleFilled(ImVec2(knob_pos_x, p.y + radius), radius - 3.0f, knob_color);

        ImVec2 text_pos = ImVec2(p.x + width + 8.0f, p.y + (height - ImGui::GetTextLineHeight()) * 0.5f);
    
        draw_list->AddText(ImVec2(text_pos.x + 1.0f, text_pos.y + 1.0f), IM_COL32(0, 0, 0, 220), label);
    
        draw_list->AddText(text_pos, IM_COL32(255, 220, 245, 255), label);

        ImGui::Dummy(ImVec2(width + 8.0f + ImGui::CalcTextSize(label).x, height));

    return clicked;
}

/* ============================================================
 *  drawUI()
 * ============================================================ */
MenuAction MenuController::drawUI(OGLRenderData& renderData) {
    MenuAction action = MenuAction::NONE;

    float panelW = 500.0f;
    float panelH = 700.0f;     float screenW = (float)renderData.rdWidth;
    float screenH = (float)renderData.rdHeight;

    ImGui::SetNextWindowPos(ImVec2(screenW * 0.08f, screenH * 0.25f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelW, panelH), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 22.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 12.0f));

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.98f, 0.94f, 0.98f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.28f, 0.08f, 0.22f, 0.70f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.82f, 0.16f, 0.55f, 0.90f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.00f, 0.25f, 0.65f, 1.00f));

    ImGui::Begin("##MainMenu", nullptr, flags);

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]);     const char* titleMain = "TENTACLE SHIPPUDEN";

    ImGui::SetWindowFontScale(1.5f);
    float titleMainW = ImGui::CalcTextSize(titleMain).x;
    ImGui::SetCursorPosX((panelW - titleMainW) * 0.5f);

    ImVec2 curPos = ImGui::GetCursorPos();

    ImGui::SetCursorPos(ImVec2(curPos.x + 2.0f, curPos.y + 2.0f));
    ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 0.9f), "%s", titleMain);

    ImGui::SetCursorPos(curPos);
    ImGui::TextColored(ImVec4(0.75f, 0.25f, 1.0f, 1.0f), "%s", titleMain);

        ImGui::SetWindowFontScale(2.0f);

    const char* wordEnty = "ENTY ";
    const char* wordFurry = "FURRY ";
    const char* wordBeast = "BEAST";

    float time = static_cast<float>(glfwGetTime()) * 2.0f;
    float hue = fmodf(time, 1.0f);
    ImVec4 furryColor = ImColor::HSV(hue, 1.0f, 1.0f);

    ImVec4 colorEnty = ImVec4(0.90f, 0.92f, 0.95f, 1.0f);
    ImVec4 colorBeast = ImVec4(1.00f, 0.15f, 0.15f, 1.0f);

    float subW = ImGui::CalcTextSize(wordEnty).x + ImGui::CalcTextSize(wordFurry).x + ImGui::CalcTextSize(wordBeast).x;
    float startX = (panelW - subW) * 0.5f;

    ImGui::SetCursorPosX(startX + 2.0f);
    curPos = ImGui::GetCursorPos();
    curPos.y += 2.0f;
    ImGui::SetCursorPos(curPos);

    ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 0.85f), "%s%s%s", wordEnty, wordFurry, wordBeast);

    ImGui::SetCursorPosX(startX);
    curPos.y -= 2.0f;
    ImGui::SetCursorPos(curPos);

    ImGui::TextColored(colorEnty, "%s", wordEnty);
    ImGui::SameLine(0, 0);

    ImGui::TextColored(furryColor, "%s", wordFurry);
    ImGui::SameLine(0, 0);

    ImGui::TextColored(colorBeast, "%s", wordBeast);

    ImGui::PopFont();
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Spacing();
    ImGui::Spacing();


    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[3]); 
    float btnW = 320.0f; 
    float btnH = 44.0f;
    float btnX = (panelW - btnW) * 0.5f; 

    bool anyHovered = false;


    ImGui::SetCursorPosX(btnX);
    if (ImGui::Button(Localization::get("start").c_str(), ImVec2(btnW, btnH))) {
        gAudio.playSound("assets/audio/Ting Coins.wav");
        gAudio.stopMusic();
        if (mAnimState != MenuAnimState::SALUTE) {
            startSalute();
        }
    }
    anyHovered |= ImGui::IsItemHovered();


    ImGui::SetCursorPosX(btnX);
    if (ImGui::Button(Localization::get("settings").c_str(), ImVec2(btnW, btnH))) {
        gAudio.playSound("assets/audio/Ting Coins.wav");
        action = MenuAction::SETTINGS;
        UserInterface::showSettingsWindow = true;
    }
    anyHovered |= ImGui::IsItemHovered();

        ImGui::SetCursorPosX(btnX);
    if (ImGui::Button("CREDITS", ImVec2(btnW, btnH))) {
        gAudio.playSound("assets/audio/Ting Coins.wav");
        renderData.showCreditsWindow = true;     }
   
    anyHovered |= ImGui::IsItemHovered();
        ImGui::SetCursorPosX(btnX);
    if (ImGui::Button(Localization::get("exit").c_str(), ImVec2(btnW, btnH))) {
        action = MenuAction::QUIT;
    }
    anyHovered |= ImGui::IsItemHovered();


    ImGui::Spacing();

        ImGui::SetCursorPosX(btnX);
    const char* tentacle = "BEAST!";
    if (CustomToggle(tentacle, &UserInterface::showTentacle, 52.0f, 26.0f)) {
            }
    anyHovered |= ImGui::IsItemHovered();

    ImGui::SetCursorPosX(btnX);
    if (CustomToggle(Localization::get("emit").c_str(), &UserInterface::showEmit, 52.0f, 26.0f)) {
            }
    ImGui::PopFont();
    anyHovered |= ImGui::IsItemHovered();

    anyHovered |= ImGui::IsItemHovered();

    ImGui::End();

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
    if (renderData.showCreditsWindow == true) {
        float panelW = 1800.0f;
        float panelH = 950.0f;
        float screenW = (float)renderData.rdWidth;
        float screenH = (float)renderData.rdHeight;

                ImGui::SetNextWindowPos(ImVec2((screenW - panelW) * 0.5f, (screenH - panelH) * 0.5f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(panelW, panelH), ImGuiCond_Always);

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.05f, 0.12f, 0.92f));

        ImGui::Begin("##CreditsMenu", nullptr, flags);

                ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]);         ImGui::SetWindowFontScale(1.3f);

        const char* title = "CREDITS & LICENSES";
        float titleW = ImGui::CalcTextSize(title).x;
        ImGui::SetCursorPosX((panelW - titleW) * 0.5f);
        ImGui::TextColored(ImVec4(0.82f, 0.16f, 0.55f, 1.0f), "%s", title);

        ImGui::PopFont();
        ImGui::SetWindowFontScale(1.0f);
        ImGui::Separator();
        ImGui::Spacing();

                        ImGui::BeginChild("CreditsScrollArea", ImVec2(0, panelH - 120.0f), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[3]); 
                ImGui::TextColored(ImVec4(0.75f, 0.25f, 1.0f, 1.0f), "   DEVELOPMENT");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Project Lead & Engine Architecture: Volkov Dinar");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "3D Art & Assets: Volkov Dinar");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Main menu guitar performer: Volkov Dinar");
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                ImGui::TextColored(ImVec4(0.75f, 0.25f, 1.0f, 1.0f), "   SPECIAL THANKS");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "AI Co-Pilots: Gemini, Grok & Claude");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "To everyone who didn't believe, betrayed, humiliated, or insulted me:");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Thank you for the motivation. STAY HARD.");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Of course the legend: Woolly Buggers 3D Animation Station for guiding me in youtube tutorial");
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                ImGui::TextColored(ImVec4(0.75f, 0.25f, 1.0f, 1.0f), "   INSPIRATIONS");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Azur lane - 3D Shipgirls");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Nier automata - UI Aesthetics");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Stellar Blade - Death Screen UI ");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Dark Souls - Boss Health Bar");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Sid Meier's Civilization V - Wonders");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "TLOU p.1 - camera stun? camera effect when fatal blow");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Conway - youtube bloger, with nice avatar ");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Agness - Umamusume");
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                ImGui::TextColored(ImVec4(0.75f, 0.25f, 1.0f, 1.0f), "    THIRD-PARTY LIBRARIES");

        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Bullet Physics SDK");
        ImGui::TextDisabled("Zlib License | Erwin Coumans");

        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Dear ImGui");
        ImGui::TextDisabled("MIT License | Omar Cornut");

        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "GLFW & GLAD");
        ImGui::TextDisabled("zlib / MIT License");

        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "GLM, tinygltf & stb_image");
        ImGui::TextDisabled("MIT / Public Domain");
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                ImGui::TextColored(ImVec4(0.75f, 0.25f, 1.0f, 1.0f), "    ASSETS, ANIMATIONS & GRAPHICS  ");

        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "3D Character Rigging & Character Animations ");
        ImGui::TextDisabled("Provided by Adobe Mixamo (mixamo.com) | Royalty-Free Commercial License");

        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "3D Nature Assets (Trees & Grass)");
        ImGui::TextDisabled("Kenney (kenney.nl) | CC0 1.0 Public Domain");

        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Cloudy Skyboxes Pack");
        ImGui::TextDisabled("Screaming Brain Studios | CC0 1.0 Public Domain");


                ImGui::TextColored(ImVec4(0.75f, 0.25f, 1.0f, 1.0f), "   Audio & Sound Effects  ");

        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Sound Effects ");
        ImGui::TextDisabled("Sound Effects provided by Sonnis (sonnis.com) | Royalty-Free Commercial License (GDC Archives)");
        ImGui::TextDisabled("Sound Effects provided by various creators on Freesound.org | Creative Commons / Public Domain Licenses");
        ImGui::TextDisabled("Some music AI downloaded and created by suno.com | non-commercial license");
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
                float windowWidth = ImGui::GetWindowSize().x;
        float textWidth = ImGui::CalcTextSize("About AI").x;
        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);

                ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.98f, 1.0f), "About AI");
        ImGui::Separator();
        ImGui::Spacing();

                ImGui::SetCursorPosX(40.0f);
        ImGui::BeginGroup();         for (int i = 1; i <= 8; ++i) {
                        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), Localization::get(std::to_string(i).c_str()).c_str());
        }
        ImGui::EndGroup();

                ImGui::Dummy(ImVec2(0.0f, 10.0f));

                ImGui::SetCursorPosX(40.0f);
        ImGui::BeginGroup();
        for (int i = 9; i <= 10; ++i) {
            ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), Localization::get(std::to_string(i).c_str()).c_str());
        }
                ImGui::Dummy(ImVec2(0.0f, 10.0f));

                ImGui::SetCursorPosX(40.0f);
        ImGui::BeginGroup();
        for (int i = 11; i <= 15; ++i) {
            ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), Localization::get(std::to_string(i).c_str()).c_str());
        }
                ImGui::Dummy(ImVec2(0.0f, 10.0f));

                ImGui::SetCursorPosX(40.0f);
        ImGui::BeginGroup();
        for (int i = 16; i <= 19; ++i) {
            ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), Localization::get(std::to_string(i).c_str()).c_str());
        }
                ImGui::Dummy(ImVec2(0.0f, 10.0f));

                ImGui::SetCursorPosX(40.0f);
        ImGui::BeginGroup();
        for (int i = 20; i <= 24; ++i) {
            ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), Localization::get(std::to_string(i).c_str()).c_str());
        }

        ImGui::EndGroup();
        ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
                float windowWidth2 = ImGui::GetWindowSize().x;
        float textWidth2 = ImGui::CalcTextSize("Cost of this pet-proect").x;
        ImGui::SetCursorPosX((windowWidth2 - textWidth2) * 0.5f);

                ImGui::TextColored(ImVec4(0.0f, 0.94f, 0.7f, 1.0f), "Cost of this pet-proect");
        ImGui::Separator();
        ImGui::Spacing();

                ImGui::SetCursorPosX(40.0f);
        ImGui::BeginGroup();         for (int i = 25; i <= 31; ++i) {
                        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), Localization::get(std::to_string(i).c_str()).c_str());
        }
        ImGui::EndGroup();

                ImGui::Dummy(ImVec2(0.0f, 10.0f));


        ImGui::PopFont();
        ImGui::EndChild(); 
        ImGui::Spacing();

                ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[3]);
        float btnW = 200.0f;
        float btnH = 40.0f;
        ImGui::SetCursorPosX((panelW - btnW) * 0.5f);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.28f, 0.08f, 0.22f, 0.80f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.82f, 0.16f, 0.55f, 0.90f));

        if (ImGui::Button("BACK", ImVec2(btnW, btnH))) {
                        gAudio.playSound("assets/audio/Ting Coins.wav");
            renderData.showCreditsWindow = false;

        }

        ImGui::PopStyleColor(2);
        ImGui::PopFont();

        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }
        float questWidth = 280.0f;
    float rightPadding = 30.0f;
    float topPadding = 25.0f;

    ImVec2 screenSize = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(screenSize.x - rightPadding, topPadding), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(questWidth, 0.0f));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 10.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 4.0f));

    if (ImGui::Begin("Feedback", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_AlwaysAutoResize))
    {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.90f, 0.98f, 0.70f));

        ImGui::SetWindowFontScale(1.15f);
        ImGui::Text("%s", Localization::get("feedback").c_str());

        ImGui::SetWindowFontScale(1.05f);
        ImGui::Text("%s", Localization::get("email").c_str());

        ImGui::PopStyleColor();
    }
    ImGui::End();

    ImGui::PopStyleVar(2);
    renderData.rdMenuAnyButtonHovered = anyHovered;

    return action;
}


glm::mat4 MenuController::getCameraViewMatrix() const {
    glm::vec3 njPos = NJ_WORLD_POS;
    glm::vec3 camPos = njPos + CAM_OFFSET;
    glm::vec3 lookAt = njPos + CAM_LOOK_OFFSET;
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    return glm::lookAt(camPos, lookAt, worldUp);
}


void MenuController::cleanup() {
    mNJInstance.reset();
    mNJModel.reset();
}


void MenuController::startCrossfade(int fromClip, int toClip) {
    mBlendSrcClip = fromClip;
    mBlendDstClip = toClip;
    mBlendFactor = 0.0f;
    mBlendTimer = 0.0f;

    auto s = mNJInstance->getInstanceSettings();

    s.msAnimClip = fromClip;
    s.msCrossBlendDestAnimClip = toClip;
    s.msBlendingMode = blendMode::crossfade;
    s.msAnimCrossBlendFactor = 0.0f;
    s.msPlayAnimation = true;
    s.msAnimSpeed = 1.0f;
    mNJInstance->setInstanceSettings(s);
    mNJInstance->checkForUpdates();
}

void MenuController::tickCrossfade(float dt) {
    mBlendTimer += dt;
    mBlendFactor = mBlendTimer / mBlendDuration;
    if (mBlendFactor > 1.0f) mBlendFactor = 1.0f;

    auto s = mNJInstance->getInstanceSettings();
    s.msAnimCrossBlendFactor = mBlendFactor;
    mNJInstance->setInstanceSettings(s);
    mNJInstance->checkForUpdates();
}

void MenuController::startSalute() {
    mSaluteTimer = 0.0f;
    mSaluteFinished = false;
    mAnimState = MenuAnimState::SALUTE;

    auto s = mNJInstance->getInstanceSettings();
    s.msAnimClip = ANIM_SALUTE;
    s.msCrossBlendDestAnimClip = ANIM_SALUTE;
    s.msBlendingMode = blendMode::crossfade;
    s.msAnimCrossBlendFactor = 1.0f;
    s.msAnimTimePosition = 0.0f;
    s.msPlayAnimation = false;  
    s.msAnimSpeed = 1.0f;
    mNJInstance->setInstanceSettings(s);
    mNJInstance->checkForUpdates();

    Logger::log(1, "%s: Salute started, duration=%.2fs\n", __FUNCTION__, mSaluteDuration);
}


void MenuController::tickSalute(float dt) {
    mSaluteTimer += dt;

    auto s = mNJInstance->getInstanceSettings();
    s.msAnimTimePosition = mSaluteTimer;
    mNJInstance->setInstanceSettings(s);
    mNJInstance->checkForUpdates();

    mNJInstance->updateAnimation();

    if (mSaluteTimer >= mSaluteDuration) {
        mSaluteFinished = true;
        Logger::log(1, "%s: Salute finished — transitioning to gameplay\n", __FUNCTION__);
    }
}