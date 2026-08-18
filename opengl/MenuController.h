// MenuController.h
#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GameStateMachine.h"
#include "JigglePhysics.h" 
// Forward declarations вместо #include "OGLRenderData.h" и "GltfModel.h"
struct OGLRenderData;
class GltfModel;
class GltfInstance;

/* ============================================================
 *  Состояния анимации NJ в меню
 * ============================================================ */
enum class MenuAnimState {
    HIPHOP,          /* танцует — курсор не на кнопках                  */
    CROSSFADE_IDLE,  /* идёт плавный переход HipHop → Idle              */
    IDLE,            /* стоит — курсор на кнопках                       */
    CROSSFADE_HH,    /* идёт плавный переход Idle → HipHop              */
    SALUTE           /* отдаёт честь (one-shot) перед запуском игры      */
};

/* ============================================================
 *  MenuController
 *
 *  Управляет:
 *    - моделью New Jersey в комнате меню
 *    - переключением анимаций с кроссфейдом
 *    - cinematic-камерой
 *    - ImGui-кнопками меню
 *
 *  Создаётся и хранится в OGLRenderer.
 *  update() вызывается каждый кадр когда GameState == MAIN_MENU.
 *  Возвращает MenuAction если игрок нажал кнопку.
 * ============================================================ */
class MenuController {
public:
    /* Вызвать один раз после загрузки модели NJ.
     * model — уже загруженная через GltfModel::loadModel() модель.   */
    bool init(std::shared_ptr<GltfModel> model);

    /* Вызывается каждый кадр в GameState::MAIN_MENU.
     * deltaTime — rdFrameTime из OGLRenderData.
     * anyButtonHovered — передать true если ImGui сообщает что курсор
     *                    находится над любой кнопкой меню.
     * Возвращает MenuAction::NONE большую часть времени.              */
    MenuAction update(float deltaTime, bool anyButtonHovered);
    MenuAction drawCreditsUI(OGLRenderData& renderData);
    /* ImGui-интерфейс меню. Вызывать внутри ImGui frame (между
     * createFrame() и render()). Возвращает то же MenuAction.         */
    MenuAction drawUI(OGLRenderData& renderData);

    /* Cinematic view matrix: камера смотрит на NJ с красивого угла.
     * Передавать напрямую в mViewMatrix вместо Camera::getViewMatrix. */
    glm::mat4 getCameraViewMatrix() const;

    /* Доступ к инстансу для рендера в основном цикле OGLRenderer.    */
    std::shared_ptr<GltfInstance> getInstance() const { return mNJInstance; }

    /* true когда анимация Salute завершилась — пора переходить в игру.*/
    bool isSaluteFinished() const { return mSaluteFinished; }

    static constexpr float ROOM_X = 0.0f;
    static constexpr float ROOM_Y = 0.0f;
    static constexpr float ROOM_Z = 0.0f;
    inline static const glm::vec3 NJ_WORLD_POS = glm::vec3(ROOM_X, ROOM_Y, ROOM_Z);

    void cleanup();
   
private:
    /* --- анимационные хелперы (паттерн из AIStates.cpp) --- */
    void startCrossfade(int fromClip, int toClip);
    void tickCrossfade(float dt);
    void startSalute();
    void tickSalute(float dt);
    void applySettings();
    BreastJiggle mLeftBreastJiggleNJ;
    BreastJiggle mRightBreastJiggleNJ;
    /* --- данные --- */
    std::shared_ptr<GltfModel>    mNJModel;
    std::shared_ptr<GltfInstance> mNJInstance;

    MenuAnimState mAnimState = MenuAnimState::HIPHOP;

    /* кроссфейд */
    int   mBlendSrcClip = 0;
    int   mBlendDstClip = 0;
    float mBlendTimer = 0.0f;
    float mBlendDuration = 1.0f;   /* секунды на кроссфейд — подбирай по вкусу */
    float mBlendFactor = 1.0f;    /* 0 = src, 1 = dst                          */

    /* salute */
    float mSaluteTimer = 0.0f;
    float mSaluteDuration = 0.0f;
    bool  mSaluteFinished = false;

    /* текущий играющий клип (для обычного loop-воспроизведения)       */
    int   mCurrentClip = 0;

    /* --------------------------------------------------------------- *
     *  АНИМАЦИОННЫЕ ИНДЕКСЫ — ПРОВЕРЬ по msClipNames своего .glb:     *
     *                                                                   *
     *    auto s = mNJInstance->getInstanceSettings();                  *
     *    for (int i=0; i<s.msClipNames.size(); i++)                   *
     *        Logger::log(1,"[%d] %s\n", i, s.msClipNames[i].c_str());*
     * --------------------------------------------------------------- */
    static constexpr int ANIM_HIPHOP = 1;
    static constexpr int ANIM_IDLE = 2;
    static constexpr int ANIM_SALUTE = 3;

    /* Комната меню — далеко от основного мира, чтобы не пересекаться. */
    /*
    static constexpr float ROOM_X = 10000.0f;
    static constexpr float ROOM_Y = -500.0f;
    static constexpr float ROOM_Z = 10000.0f;
    */
    

    /* Позиция и поворот NJ в комнате меню.                             */
   
    const glm::vec3 NJ_WORLD_ROT = glm::vec3(0.0f, 30.0f, 0.0f);

    /* Cinematic camera: немного справа, на уровне груди, смотрит вниз-внутрь */
    const glm::vec3 CAM_OFFSET = glm::vec3(1.8f, 1.6f, 2.5f);
    const glm::vec3 CAM_LOOK_OFFSET = glm::vec3(0.0f, 1.8f, 0.0f);
};