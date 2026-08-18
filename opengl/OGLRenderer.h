#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <vector>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "JigglePhysics.h"
#include "Timer.h"
#include "Framebuffer.h"
#include "VertexBuffer.h"
#include "Texture.h"
#include "Shader.h"
#include "UniformBuffer.h"
#include "ShaderStorageBuffer.h"
#include "UserInterface.h"
#include "Camera.h"
#include "CoordArrowsModel.h"
#include "GltfModel.h"
#include "GltfInstance.h"

#include "OGLRenderData.h"
#include "btBulletDynamicsCommon.h"
#include "Enemy.h"
#include "Bossconway.h"
#include "Bossbismark.h"
#include "SkyBox.h"
#include "GraphicsManager.h"
#include <cmath>   // std::atan2 — если ещё нет
#include "GameStateMachine.h"
#include "MenuController.h"

enum class AgnessState {
    Sitting,   // клип 3 — сидит/ждёт
    StandUp,   // клип 4 — встаёт, один раз
    Idle       // клип 1 — стоит, зациклено
};

enum  EntyAnim : int {
    KICK = 11, //
    Action = 0, // стоит не в Т-позе
    Dance1 = 2,
    Boxing = 1,
    DashBack = 4,
    Death = 5,
    DoubleJumpKick = 6,
    FlyKick = 7,
    HitReaction = 8,
    Idle = 9,
    Jump = 10,
    PickUp = 12,
    Run = 13,
    StartWalk = 14,
    StopWalk = 18,
    StopRun = 15,
    Tpose = 16,
    Walk = 17,
    DanceWin = 3
};

class OGLRenderer {
public:
    OGLRenderer(GLFWwindow* window);
    void updateAgness(float deltaTime, const glm::vec3& playerPos);
    bool init(unsigned int width, unsigned int height);
    void setSize(unsigned int width, unsigned int height);
    void uploadData(OGLMesh vertexData);
    void GltfGPUShaderToy(unsigned int matrixPos);
    void GltfGPUShaderToyChannelPack(unsigned int matrixPos, const std::shared_ptr<GltfInstance>& instance);
    void draw();
    void handleKeyEvents(int key, int scancode, int action, int mods);
    void handleMouseButtonEvents(int button, int action, int mods);
    void handleMousePositionEvents(double xPos, double yPos);
    
    void addPhysicsToEnty(std::shared_ptr<GltfInstance> entyInstance, btDiscreteDynamicsWorld* dynamicsWorld, glm::vec3 spawnPos);
    void addPhysicsToEnemy(std::shared_ptr<GltfInstance> entyInstance, btDiscreteDynamicsWorld* dynamicsWorld, glm::vec3 spawnPos);
    void addEnemy(std::shared_ptr<GltfModel> model, int x, int y, int z);
    void addBossConway(std::shared_ptr<GltfModel> model, int x, int y, int z);
    void addBossBismark(std::shared_ptr<GltfModel> model, int x, int y, int z);
    void addPhysicsToStonehenge(std::shared_ptr<GltfInstance> stonehengeInstance, btDiscreteDynamicsWorld* dynamicsWorld, glm::vec3 pos, glm::vec3 rot);
    void updatePlayerCharacter(float deltaTime);

    void CheckPlayerMeleeHit(float hitRadius, float damage);
    void cleanup();
    void ResetPlayerAfterDeath();
    OGLRenderData& getRenderData() { return mRenderData; }
private:

    // --- ПЕРЕМЕННЫЕ BULLET PHYSICS ---
    btDefaultCollisionConfiguration* mCollisionConfiguration = nullptr;
    btCollisionDispatcher* mDispatcher = nullptr;
    btBroadphaseInterface* mOverlappingPairCache = nullptr;
    btSequentialImpulseConstraintSolver* mSolver = nullptr;
    btDiscreteDynamicsWorld* mDynamicsWorld = nullptr;

    OGLRenderData mRenderData{};
    bool mShowMenu = false;
    Timer mFrameTimer{};
    Timer mMatrixGenerateTimer{};
    Timer mIKTimer{};
    Timer mUploadToVBOTimer{};
    Timer mUploadToUBOTimer{};
    Timer mUIGenerateTimer{};
    Timer mUIDrawTimer{};

    Shader mLineShader{}; // Для отрисовки скелета и стрелочек координат
    Shader mGltfGPUShader{}; // Для стандартного скиннинга на GPU (матрицы)
    Shader mGltfGPUDualQuatShader{}; // Для скиннинга через дуальные кватернионы
    Shader mStaticShader{}; // Для статичных объектов без костей (храмы, арены) mStaticShader.loadShaders("shader/gltf.vert", "shader/gltf.frag")) {
    Shader mGltfGPUShaderToy{}; // Для тестирования нового шейдера на основе старого гпу для костлявых
    Shader mGltfGPUShaderToyChannelPack{}; // Для тестирования нового шейдера на основе старого гпу для костлявых
    Shader mGltfGPUDualQuatChannelShader{};

    Framebuffer mFramebuffer{};
    MSFramebuffer mMSFramebuffer;
    GraphicsManager mGraphicsManager;

    VertexBuffer mVertexBuffer{};
    UniformBuffer mUniformBuffer{};
    ShaderStorageBuffer mGltfShaderStorageBuffer{};
    ShaderStorageBuffer mGltfDualQuatSSBuffer{};
    UserInterface mUserInterface{};
    Camera mCamera{};

    //GameState mGameState = GameState::MAIN_MENU;
    GameState mGameState = GameState::MAIN_MENU;
    std::unique_ptr<MenuController> mMenuController;
    std::shared_ptr<GltfModel>      mNJMenuModel;   // модель NJ только для меню

    SkyBox mSkyBox{};
    float mAccumulatedTime = 0.0f;   // секунды с момента старта
    // Такой же тип, как mBlurFBO[0] / mFramebuffer — подставьте ваш реальный класс
    Framebuffer mDeathCaptureFBO;   // сюда уходит финальный composite вместо экрана
    Framebuffer mDeathGlowFBO[2];   // ping-pong для блюра HDR-кнопки (можно переиспользовать mBlurFBO, см. ниже)

    // Bloom: ping-pong blur FBO (два буфера чередуются)
    BlurFramebuffer mBlurFBO[2]{};

    BreastJiggle mLeftBreastJiggle;
    BreastJiggle mRightBreastJiggle;
    // Bloom шейдеры
    Shader mBloomBlurShader{};       // bloom_blur.vert / bloom_blur.frag
    Shader mBloomCompositeShader{};  // bloom_blur.vert / bloom_composite.frag
    Shader mDeathGlowShader;          // DeathScreen_Glow.frag + ваш Bloom_Blur.vert
    Shader mDeathGlowCompositeShader; // DeathScreen_GlowComposite.frag + Bloom_Blur.vert
    // Настройки bloom
    float mBloomStrength = 1.0f;  // итоговый множитель bloom
    float mEmitStrength = 2.5f;  // множитель emission в geometry шейдерах
    int   mBloomPasses = 10;    // кол-во ping-pong итераций (чётное число)

    // VAO для fullscreen triangle (без VBO — используем gl_VertexID)
    GLuint mBloomVAO = 0;

    std::vector<std::shared_ptr<GltfModel>> mGltfModels{};

    std::vector<std::shared_ptr<GltfInstance>> mGltfInstances{};
    std::vector<std::shared_ptr<GltfInstance>> mGltfMatrixInstances{};
    std::vector<std::shared_ptr<GltfInstance>> mGltfDQInstances{};
    //24.01
    std::vector<std::shared_ptr<GltfInstance>> mStaticInstances;

    std::vector<glm::mat4> mModelJointMatrices{};
    std::vector<glm::mat2x4> mModelJointDualQuats{};

    CoordArrowsModel mCoordArrowsModel{};
    OGLMesh mCoordArrowsMesh{};
    std::shared_ptr<OGLMesh> mLineMesh = nullptr;
    unsigned int mSkeletonLineIndexCount = 0;
    unsigned int mCoordArrowsLineIndexCount = 0;

    bool mMouseLock = false;
    int mMouseXPos = 0;
    int mMouseYPos = 0;

    double mLastTickTime = 0.0;

    void handleMovementKeys();

    /* create identity matrix by default */
    glm::mat4 mViewMatrix = glm::mat4(1.0f);
    glm::mat4 mProjectionMatrix = glm::mat4(1.0f);

    bool mWasSpacePressed = false;
   // std::vector<std::unique_ptr<Enemy>> mEnemies; OOP
    std::vector<std::unique_ptr<Enemy>> mEnemies; // DOD - Data-Oriented Design (ориентированное на данные проектирование)
    bool mIsEntyStunned = false;
    float mEntyStunTimer = 0.0f;
    float mEntyStunDuration = 1.13f;

    /* Состояния перехода (отдельно от GameState — fade идёт поверх любого состояния) */
    enum class FadeState {
        NONE,           /* нет оверлея, геймплей виден нормально        */
        FADE_OUT,       /* затемнение 0→1 после окончания Salute         */
        LOADING_WAIT,   /* экран чёрный, спиннер, камера уже телепортирована */
        FADE_IN         /* осветление 1→0 перед геймплеем               */
    };

    FadeState   mFadeState = FadeState::NONE;
    float       mFadeAlpha = 0.0f;   /* 0 = прозрачно, 1 = чёрный               */
    float       mLoadingTimer = 0.0f;   /* сколько секунд висим на экране загрузки  */
    float       mLoadingSpinAngle = 0.0f;   /* угол спиннера, накапливается каждый кадр */

    /* Настройки — меняй по вкусу */
    static constexpr float FADE_DURATION = 0.55f;  /* секунды затемнения/осветления    */
    static constexpr float LOADING_MIN_TIME = 0.8f;   /* мин. время на экране загрузки    */

    // --- Death Screen (Stellar Blade style) ---
    bool  mShowDeathScreen = false;
    bool  mRespawnRequested = false;
    float mDeathFadeAlpha = 0.0f;   // 0.0 .. DEATH_FADE_MAX, blur+darken
    float mRespawnGlowPulse = 0.0f;   // для лёгкой пульсации неона кнопки
    static constexpr float DEATH_FADE_DURATION = 1.4f;  // сек. до полного затемнения
    static constexpr float DEATH_FADE_MAX = 0.85f;

    // --- Boss Health Bar (DS3 / Elden Ring style) ---
    Enemy* mActiveBossEnemy = nullptr;     // не владеющий указатель — жизнь боссов у mEnemies
    float  mBossBarDisplayHealth = 0.0f;   // "хвостовая" белая полоса чип-урона (0..1)
    float  mBossBarVisibleAlpha = 0.0f;   // плавное появление/исчезание всей полосы
    AgnessState mAgnessState = AgnessState::Sitting;
    float mAgnessStandUpTimer = 0.0f;
};


