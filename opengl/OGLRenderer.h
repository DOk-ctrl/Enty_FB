#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <vector>
#include <string>
#include <future>
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

#include "GltfModel.h"
#include "GltfInstance.h"

#include "OGLRenderData.h"
#include "btBulletDynamicsCommon.h"
#include "Enemy.h"
#include "Bossconway.h"
#include "Bossbismark.h"
#include "SkyBox.h"
#include "GraphicsManager.h"
#include <cmath>   
#include "GameStateMachine.h"
#include "MenuController.h"


enum class LoadState { NotStarted, LoadingData, UploadingToGPU, BuildingWorld, Ready, Failed };

enum class AgnessState {
    Sitting,   
    StandUp,   
    Idle       
};

enum  EntyAnim : int {
    KICK = 11,     Action = 0,     Dance1 = 2,
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
    void updateAgness(float deltaTime, const glm::vec3& playerPos, OGLRenderData& renderData);
    bool init(unsigned int width, unsigned int height);
    void setSize(unsigned int width, unsigned int height);
    void uploadData(OGLMesh vertexData);
    void GltfGPUShaderToy(unsigned int matrixPos);
    void GltfGPUShaderToyChannelPack(unsigned int matrixPos, const std::shared_ptr<GltfInstance>& instance);
    void draw();
    void handleKeyEvents(int key, int scancode, int action, int mods);
    void handleMouseButtonEvents(int button, int action, int mods);
    void handleMousePositionEvents(double xPos, double yPos);
    
    bool initEarly(unsigned int width, unsigned int height);
    void updateLoading();               // вызывать раз в кадр, пока !isReady()
    void drawLoadingScreen();           // простой ImGui progress bar
    bool isReady() const { return mLoadState == LoadState::Ready; }
    bool loadFailed() const { return mLoadState == LoadState::Failed; }

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

    struct ModelLoadInfo {
        std::string modelFile, texFile, normalFile, metalFile, roughFile, emitFile, packedFile, coatRoughFile;
    };

    void startBackgroundLoading();      // запускает std::async на все модели
    void buildWorld();                  // старый "хвост" — инстансы/физика/враги/боссы/SSBO

    LoadState mLoadState = LoadState::NotStarted;
    std::vector<ModelLoadInfo> mModelList;
    std::vector<std::future<bool>> mModelLoaders;
    std::vector<bool> mModelLoadDone;
    std::vector<bool> mModelLoadSuccess;
    size_t mUploadIndex = 0;

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

    Shader mLineShader{};     Shader mGltfGPUShader{};     Shader mGltfGPUDualQuatShader{};     Shader mStaticShader{};     Shader mGltfGPUShaderToy{};     Shader mGltfGPUShaderToyChannelPack{};     Shader mGltfGPUDualQuatChannelShader{};

    Framebuffer mFramebuffer{};
    MSFramebuffer mMSFramebuffer;
    GraphicsManager mGraphicsManager;

    VertexBuffer mVertexBuffer{};
    UniformBuffer mUniformBuffer{};
    ShaderStorageBuffer mGltfShaderStorageBuffer{};
    ShaderStorageBuffer mGltfDualQuatSSBuffer{};
    UserInterface mUserInterface{};
    Camera mCamera{};

        GameState mGameState = GameState::MAIN_MENU;
    std::unique_ptr<MenuController> mMenuController;
    std::shared_ptr<GltfModel>      mNJMenuModel;   
    SkyBox mSkyBox{};
    float mAccumulatedTime = 0.0f;           Framebuffer mDeathCaptureFBO;       Framebuffer mDeathGlowFBO[2];   
        BlurFramebuffer mBlurFBO[2]{};

    BreastJiggle mLeftBreastJiggle;
    BreastJiggle mRightBreastJiggle;
        Shader mBloomBlurShader{};           Shader mBloomCompositeShader{};      Shader mDeathGlowShader;              Shader mDeathGlowCompositeShader;         float mBloomStrength = 1.0f;      float mEmitStrength = 2.5f;      int   mBloomPasses = 10;    
        GLuint mBloomVAO = 0;

    std::vector<std::shared_ptr<GltfModel>> mGltfModels{};

    std::vector<std::shared_ptr<GltfInstance>> mGltfInstances{};
    std::vector<std::shared_ptr<GltfInstance>> mGltfMatrixInstances{};
    std::vector<std::shared_ptr<GltfInstance>> mGltfDQInstances{};
        std::vector<std::shared_ptr<GltfInstance>> mStaticInstances;

    std::vector<glm::mat4> mModelJointMatrices{};
    std::vector<glm::mat2x4> mModelJointDualQuats{};

    std::vector<std::future<TextureLoadResult>> pendingTextures;

    std::shared_ptr<OGLMesh> mLineMesh = nullptr;
    unsigned int mSkeletonLineIndexCount = 0;

    bool mMouseLock = false;
    int mMouseXPos = 0;
    int mMouseYPos = 0;

    double mLastTickTime = 0.0;

    void handleMovementKeys();

    /* create identity matrix by default */
    glm::mat4 mViewMatrix = glm::mat4(1.0f);
    glm::mat4 mProjectionMatrix = glm::mat4(1.0f);

    bool mWasSpacePressed = false;
       std::vector<std::unique_ptr<Enemy>> mEnemies;     bool mIsEntyStunned = false;
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

        bool  mShowDeathScreen = false;
    bool  mRespawnRequested = false;
    float mDeathFadeAlpha = 0.0f;       float mRespawnGlowPulse = 0.0f;       static constexpr float DEATH_FADE_DURATION = 1.4f;      static constexpr float DEATH_FADE_MAX = 0.85f;

        Enemy* mActiveBossEnemy = nullptr;         float  mBossBarDisplayHealth = 0.0f;       float  mBossBarVisibleAlpha = 0.0f;       AgnessState mAgnessState = AgnessState::Sitting;
    float mAgnessStandUpTimer = 0.0f;

    Enemy* mBismarkBossPtr = nullptr;       bool   mBismarkGateCollisionActive = true; };


