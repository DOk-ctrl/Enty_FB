// MenuController.cpp
#define IMGUI_DEFINE_MATH_OPERATORS // <--- ДОБАВИТЬ СТРОГО ПЕРЕД imgui.h
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
// #include <imgui.cpp>  <--- ЭТУ СТРОКУ УДАЛИ
/* ============================================================
 *  init()
 * ============================================================ */
bool MenuController::init(std::shared_ptr<GltfModel> model) {
    if (!model) {
        Logger::log(1, "%s error: null GltfModel passed\n", __FUNCTION__);
        return false;
    }
    mNJModel = model;

    /* Создаём инстанс — сразу ставим в комнату меню */
    mNJInstance = std::make_shared<GltfInstance>(mNJModel, NJ_WORLD_POS, NJ_WORLD_ROT);
    {
        auto root = mNJInstance->getRootNode();

        // Разово проверить реальные имена костей в логе — Mixamo иногда
        // переименовывает кости при экспорте (см. "mixamorig:Hips" в клипах).
        // Если "Breast.L"/"Breast.R" внизу не найдутся — раскомментируй и глянь лог:
        // root->printTree();

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

    /* Сразу запускаем HipHop */
    {
        auto s = mNJInstance->getInstanceSettings();
        s.msWorldPosition = NJ_WORLD_POS;
        s.msWorldRotation = NJ_WORLD_ROT;

        s.msAnimClip = ANIM_HIPHOP;
        s.msCrossBlendDestAnimClip = ANIM_HIPHOP;
        s.msBlendingMode = blendMode::crossfade;
        s.msAnimCrossBlendFactor = 1.0f;   /* мгновенно на HipHop */
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


/* ============================================================
 *  drawUI()  — ImGui кнопки меню
 * ============================================================ */
 /* ============================================================
  *  Хелпер кастомного Toggle в стиле MiSide (Капсула + Плавная анимация)
  * ============================================================ */
  /* ============================================================
   *  Хелпер кастомного Toggle (100% Public ImGui API)
   * ============================================================ */
bool CustomToggle(const char* label, bool* v, float width = 52.0f, float height = 26.0f) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Создаем невидимую кнопку под размер капсулы
    ImGui::InvisibleButton(label, ImVec2(width, height));
    bool clicked = ImGui::IsItemClicked();
    bool hovered = ImGui::IsItemHovered();

    if (clicked) {
        *v = !(*v);
    }

    // Анимация смещения (плавный переход 0.0f -> 1.0f)
    float t = *v ? 1.0f : 0.0f;
    float anim_speed = 12.0f;

    // Используем уникальный ID кнопки для сохранения состояния анимации
    ImGuiID id = ImGui::GetID(label);
    float t_anim = ImGui::GetStateStorage()->GetFloat(id, t);

    // Плавная интерполяция
    float dt = ImGui::GetIO().DeltaTime;
    if (t_anim < t) t_anim = (t_anim + dt * anim_speed < t) ? t_anim + dt * anim_speed : t;
    else if (t_anim > t) t_anim = (t_anim - dt * anim_speed > t) ? t_anim - dt * anim_speed : t;

    ImGui::GetStateStorage()->SetFloat(id, t_anim);

    // Цвета стиля
    ImU32 bg_color = hovered
        ? ImColor(0.82f, 0.16f, 0.55f, 0.90f)   // Hover
        : (*v ? ImColor(0.65f, 0.15f, 0.50f, 0.85f)  // ON
            : ImColor(0.28f, 0.08f, 0.22f, 0.70f)); // OFF

    ImU32 knob_color = ImColor(0.98f, 0.94f, 0.98f, 1.00f);

    float radius = height * 0.5f;

    // Отрисовка фона капсулы
    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), bg_color, radius);

    // Отрисовка ползунка
    float knob_pos_x = p.x + radius + t_anim * (width - radius * 2.0f);
    draw_list->AddCircleFilled(ImVec2(knob_pos_x, p.y + radius), radius - 3.0f, knob_color);

    // Текст рядом с тогглом (с тенью для читаемости на синем фоне)
    ImVec2 text_pos = ImVec2(p.x + width + 8.0f, p.y + (height - ImGui::GetTextLineHeight()) * 0.5f);
    
    // 1. Четкая тень
    draw_list->AddText(ImVec2(text_pos.x + 1.0f, text_pos.y + 1.0f), IM_COL32(0, 0, 0, 220), label);
    
    // 2. Сам текст (можно поменять цвет: сейчас стоит нежно-розовый/белый под тему)
    draw_list->AddText(text_pos, IM_COL32(255, 220, 245, 255), label);

    // Смещаем курсор ImGui дальше вправо, чтобы он понимал, где кончился элемент
    ImGui::Dummy(ImVec2(width + 8.0f + ImGui::CalcTextSize(label).x, height));

    return clicked;
}

/* ============================================================
 *  drawUI()
 * ============================================================ */
MenuAction MenuController::drawUI(OGLRenderData& renderData) {
    MenuAction action = MenuAction::NONE;

    float panelW = 500.0f;
    float panelH = 700.0f; // Увеличили высоту панели, чтобы влез тоггл
    float screenW = (float)renderData.rdWidth;
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

    // --- БЛОК ЗАГОЛОВКА ---
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]); // Включаем Monomaniac One
    const char* titleMain = "TENTACLE SHIPPUDEN";

    ImGui::SetWindowFontScale(1.5f);
    float titleMainW = ImGui::CalcTextSize(titleMain).x;
    ImGui::SetCursorPosX((panelW - titleMainW) * 0.5f);

    ImVec2 curPos = ImGui::GetCursorPos();

    ImGui::SetCursorPos(ImVec2(curPos.x + 2.0f, curPos.y + 2.0f));
    ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 0.9f), "%s", titleMain);

    ImGui::SetCursorPos(curPos);
    ImGui::TextColored(ImVec4(0.75f, 0.25f, 1.0f, 1.0f), "%s", titleMain);

    // 2. Подзаголовок: ENTY FURRY BEAST
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

    ImGui::PopFont(); // Возвращаемся на Arial для кнопок
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Spacing();
    ImGui::Spacing();

    /* --- КНОПКИ --- */
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[3]); // Переключаемся на ExtraBold для кнопок!
    float btnW = 320.0f; // Фиксированная аккуратная ширина кнопок
    float btnH = 44.0f;
    float btnX = (panelW - btnW) * 0.5f; // Позиция для центрирования

    bool anyHovered = false;

    // Функция-помощник для перевода строки в ВЕРХНИЙ РЕГИСТР (CAPS)
    
    // Кнопка 1
    ImGui::SetCursorPosX(btnX);
    if (ImGui::Button(Localization::get("start").c_str(), ImVec2(btnW, btnH))) {
        if (mAnimState != MenuAnimState::SALUTE) {
            startSalute();
        }
    }
    anyHovered |= ImGui::IsItemHovered();

    // Кнопка 2
    ImGui::SetCursorPosX(btnX);
    if (ImGui::Button(Localization::get("settings").c_str(), ImVec2(btnW, btnH))) {
        action = MenuAction::SETTINGS;
        UserInterface::showSettingsWindow = true;
    }
    anyHovered |= ImGui::IsItemHovered();

    // Кнопка CREDITS (Внутриигровые титры)
    ImGui::SetCursorPosX(btnX);
    if (ImGui::Button("CREDITS", ImVec2(btnW, btnH))) {
        renderData.showCreditsWindow = true; // Открываем внутриигровое окно титров
    }
   
    anyHovered |= ImGui::IsItemHovered();
    // Кнопка 3
    ImGui::SetCursorPosX(btnX);
    if (ImGui::Button(Localization::get("exit").c_str(), ImVec2(btnW, btnH))) {
        action = MenuAction::QUIT;
    }
    anyHovered |= ImGui::IsItemHovered();

    /* --- КАСТОМНЫЕ TOGGLES ПОД ВЫХОДОМ --- */
    ImGui::Spacing();

    // Тогглы тоже центрируем или выравниваем по левому краю кнопок:
    ImGui::SetCursorPosX(btnX);
    const char* tentacle = "TENTACLE!";
    if (CustomToggle(tentacle, &UserInterface::showTentacle, 52.0f, 26.0f)) {
        // Логика
    }
    anyHovered |= ImGui::IsItemHovered();

    ImGui::Spacing();

    ImGui::SetCursorPosX(btnX);
    if (CustomToggle(Localization::get("emit").c_str(), &UserInterface::showEmit, 52.0f, 26.0f)) {
        // Логика
    }
    ImGui::PopFont();
    anyHovered |= ImGui::IsItemHovered();

    anyHovered |= ImGui::IsItemHovered();

    ImGui::End();

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
    if (renderData.showCreditsWindow == true) {
        float panelW = 900.0f;
        float panelH = 950.0f;
        float screenW = (float)renderData.rdWidth;
        float screenH = (float)renderData.rdHeight;

        // Центрируем окно титров на экране
        ImGui::SetNextWindowPos(ImVec2((screenW - panelW) * 0.5f, (screenH - panelH) * 0.5f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(panelW, panelH), ImGuiCond_Always);

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        // Темно-фиолетовый фон с прозрачностью, под стиль твоего меню
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.05f, 0.12f, 0.92f));

        ImGui::Begin("##CreditsMenu", nullptr, flags);

        // --- ЗАГОЛОВК ---
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]); // Monomaniac One
        ImGui::SetWindowFontScale(1.3f);

        const char* title = "CREDITS & LICENSES";
        float titleW = ImGui::CalcTextSize(title).x;
        ImGui::SetCursorPosX((panelW - titleW) * 0.5f);
        ImGui::TextColored(ImVec4(0.82f, 0.16f, 0.55f, 1.0f), "%s", title);

        ImGui::PopFont();
        ImGui::SetWindowFontScale(1.0f);
        ImGui::Separator();
        ImGui::Spacing();

        // --- СКРОЛЛИРУЕМАЯ ОБЛАСТЬ С ТИТРАМИ ---
        // Оставляем снизу место под кнопку "Назад" (около 50px)
        ImGui::BeginChild("CreditsScrollArea", ImVec2(0, panelH - 120.0f), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[3]); // ExtraBold / основной шрифт

        // Раздел 1: Разработчики
        ImGui::TextColored(ImVec4(0.75f, 0.25f, 1.0f, 1.0f), "DEVELOPMENT");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Project Lead & Engine Architecture: Volkov Dinar");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "3D Art & Assets: Volkov Dinar");
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

        // Раздел 2: Источники вдохновения
        ImGui::TextColored(ImVec4(0.75f, 0.25f, 1.0f, 1.0f), "INSPIRATIONS");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "AZur lane - 3D Shipgirls");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Nier automata - UI Aesthetics");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Stellar Blade - Death Screen UI ");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Dark Souls - Boss Health Bar");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Sid Meier's Civilization V — Wonders");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "TLOU p.1 — camera stun? camera effect when fatal blow");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Conway — ну этот чел  с ютуба с этим аватаром клоунихи");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Agness — Umamusume не смотрел: просто сказали сделать, я сделал");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "7DTD - ");
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

        // Раздел 3: Особые благодарности
        ImGui::TextColored(ImVec4(0.75f, 0.25f, 1.0f, 1.0f), "SPECIAL THANKS");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "AI Co-Pilots: Gemini, Grok & Claude");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "David Goggins - for the podcasts");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "To everyone who didn't believe, betrayed, humiliated, or insulted me:");
        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Thank you for the motivation. STAY HARD.");
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

        // Раздел 2: Сторонние библиотеки (C++)
        ImGui::TextColored(ImVec4(0.75f, 0.25f, 1.0f, 1.0f), "=== THIRD-PARTY LIBRARIES ===");

        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Bullet Physics SDK");
        ImGui::TextDisabled("Zlib License | Erwin Coumans");

        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Dear ImGui");
        ImGui::TextDisabled("MIT License | Omar Cornut");

        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "GLFW & GLAD");
        ImGui::TextDisabled("zlib / MIT License");

        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "GLM, tinygltf & stb_image");
        ImGui::TextDisabled("MIT / Public Domain");
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

        // Раздел 3: Графика и Ассеты
        ImGui::TextColored(ImVec4(0.75f, 0.25f, 1.0f, 1.0f), "=== ASSETS & GRAPHICS ===");

        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "3D Nature Assets (Trees & Grass)");
        ImGui::TextDisabled("Kenney (kenney.nl) | CC0 1.0 Public Domain");

        ImGui::TextColored(ImVec4(0.98f, 0.94f, 0.98f, 1.0f), "Cloudy Skyboxes Pack");
        ImGui::TextDisabled("Screaming Brain Studios | CC0 1.0 Public Domain");

        ImGui::PopFont();
        ImGui::EndChild(); // Конец скролл-области

        ImGui::Spacing();

        // --- КНОПКА "НАЗАД" ---
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[3]);
        float btnW = 200.0f;
        float btnH = 40.0f;
        ImGui::SetCursorPosX((panelW - btnW) * 0.5f);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.28f, 0.08f, 0.22f, 0.80f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.82f, 0.16f, 0.55f, 0.90f));

        if (ImGui::Button("BACK", ImVec2(btnW, btnH))) {
            // Переключаем обратно на главное меню
            renderData.showCreditsWindow = false;

        }

        ImGui::PopStyleColor(2);
        ImGui::PopFont();

        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }
    renderData.rdMenuAnyButtonHovered = anyHovered;

    return action;
}

/* ============================================================
 *  getCameraViewMatrix()
 * ============================================================ */
glm::mat4 MenuController::getCameraViewMatrix() const {
    glm::vec3 njPos = NJ_WORLD_POS;
    glm::vec3 camPos = njPos + CAM_OFFSET;
    glm::vec3 lookAt = njPos + CAM_LOOK_OFFSET;
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    return glm::lookAt(camPos, lookAt, worldUp);
}

/* ============================================================
 *  cleanup()
 * ============================================================ */
void MenuController::cleanup() {
    mNJInstance.reset();
    mNJModel.reset();
}

/* ============================================================
 *  Приватные хелперы
 * ============================================================ */

 /* Начать кроссфейд из fromClip в toClip */
void MenuController::startCrossfade(int fromClip, int toClip) {
    mBlendSrcClip = fromClip;
    mBlendDstClip = toClip;
    mBlendFactor = 0.0f;
    mBlendTimer = 0.0f;

    auto s = mNJInstance->getInstanceSettings();
    /* source clip продолжает играть, dest начнёт появляться по мере роста factor */
    s.msAnimClip = fromClip;
    s.msCrossBlendDestAnimClip = toClip;
    s.msBlendingMode = blendMode::crossfade;
    s.msAnimCrossBlendFactor = 0.0f;
    s.msPlayAnimation = true;
    s.msAnimSpeed = 1.0f;
    mNJInstance->setInstanceSettings(s);
    mNJInstance->checkForUpdates();
}

/* Тикать кроссфейд каждый кадр — передавать deltaTime */
void MenuController::tickCrossfade(float dt) {
    mBlendTimer += dt;
    mBlendFactor = mBlendTimer / mBlendDuration;
    if (mBlendFactor > 1.0f) mBlendFactor = 1.0f;

    auto s = mNJInstance->getInstanceSettings();
    s.msAnimCrossBlendFactor = mBlendFactor;
    mNJInstance->setInstanceSettings(s);
    mNJInstance->checkForUpdates();
}

/* Запустить Salute (one-shot, ручной тайминг — как AIStateAttack) */
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
    s.msPlayAnimation = false;  /* ручной контроль времени */
    s.msAnimSpeed = 1.0f;
    mNJInstance->setInstanceSettings(s);
    mNJInstance->checkForUpdates();

    Logger::log(1, "%s: Salute started, duration=%.2fs\n", __FUNCTION__, mSaluteDuration);
}

/* Обновлять таймер Salute (паттерн из AIStateAttack::update) */
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