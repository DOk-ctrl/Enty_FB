#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <vector>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
enum  EntyAnim : int {
    KICK = 0, //
    Action = 1, // ÒÚÓËÚ ÌÂ ‚ “-ÔÓÁÂ
    Dance1 = 2,
    Boxing = 3,
    DashBack = 4,
    Death = 5,
    DoubleJumpKick = 6,
    FlyKick = 7,
    HitReaction = 8,
    Idle = 9,
    Jump = 10,
    PickUp = 11,
    Run = 12,
    StartWalk = 13,
    StopWalk = 14,
    StopRun = 15,
    Tpose = 16,
    Walk = 17,
    DanceWin = 18
};

class OGLRenderer {
  public:
    OGLRenderer(GLFWwindow *window);

    bool init(unsigned int width, unsigned int height);
    void setSize(unsigned int width, unsigned int height);
    void uploadData(OGLMesh vertexData);
    void draw();
    void handleKeyEvents(int key, int scancode, int action, int mods);
    void handleMouseButtonEvents(int button, int action, int mods);
    void handleMousePositionEvents(double xPos, double yPos);

    void addPhysicsToEnty(std::shared_ptr<GltfInstance> entyInstance, btDiscreteDynamicsWorld* dynamicsWorld, glm::vec3 spawnPos);
    void addPhysicsToEnemy(std::shared_ptr<GltfInstance> entyInstance, btDiscreteDynamicsWorld* dynamicsWorld, glm::vec3 spawnPos);
    void addPhysicsToStonehenge(std::shared_ptr<GltfInstance> stonehengeInstance, btDiscreteDynamicsWorld* dynamicsWorld, glm::vec3 pos, glm::vec3 rot);
    void updatePlayerCharacter(float deltaTime);

    void cleanup();


  private:

    // --- œ≈–≈Ã≈ÕÕ€≈ BULLET PHYSICS ---
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

    Shader mLineShader{};
    Shader mGltfGPUShader{};
    Shader mGltfGPUDualQuatShader{};
    Shader mStaticShader{};

    Framebuffer mFramebuffer{};
    VertexBuffer mVertexBuffer{};
    UniformBuffer mUniformBuffer{};
    ShaderStorageBuffer mGltfShaderStorageBuffer{};
    ShaderStorageBuffer mGltfDualQuatSSBuffer{};
    UserInterface mUserInterface{};
    Camera mCamera{};

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
    std::vector<std::unique_ptr<Enemy>> mEnemies;

};


