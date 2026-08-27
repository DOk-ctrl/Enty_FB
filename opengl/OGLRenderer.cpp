#define GLM_ENABLE_EXPERIMENTAL
#define _USE_MATH_DEFINES
#include <algorithm>

#include <imgui_impl_glfw.h>

#include <glm/gtc/matrix_transform.hpp>

#include <ctime>
#include <cstdlib>

#include "OGLRenderer.h"
#include "ModelSettings.h"
#include "Logger.h"
#include "UserInterface.h"
#include "SkyBox.h"
#include "audio.h"
#include <iostream>
#include <imgui_internal.h>
#include <random> 
static std::mt19937 rngMeleeCrit(std::random_device{}());
static std::uniform_real_distribution<float> critDist(0.0f, 1.0f);

OGLRenderer::OGLRenderer(GLFWwindow *window) {
  mRenderData.rdWindow = window;
}

void OGLRenderer::updateAgness(float deltaTime, const glm::vec3& playerPos, OGLRenderData& renderData)
{
    auto& agness = mGltfInstances.at(9);
    btTransform trans;
    agness->getRigidBody()->getMotionState()->getWorldTransform(trans);
    glm::vec3 agnessPos(trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z());

    float distance = glm::length(playerPos - agnessPos);
    const float triggerRadius = 8.0f; 
    switch (mAgnessState) {
    case AgnessState::Sitting: {
        if (distance <= triggerRadius) {
            gAudio.playMusic("assets/audio/wonmusic.MP3");

                        gAudio.playSound("assets/audio/win.wav");
            ModelSettings s = agness->getInstanceSettings();
            s.msAnimClip = 4;
            s.msCrossBlendDestAnimClip = 4;
            s.msBlendingMode = blendMode::crossfade;
            s.msAnimCrossBlendFactor = 1.0f;
            s.msAnimTimePosition = 0.0f;
            s.msPlayAnimation = false;             agness->setInstanceSettings(s);
            agness->checkForUpdates();

            mAgnessStandUpTimer = 0.0f;
            mAgnessState = AgnessState::StandUp;
            renderData.rdIsGameWon = true;
        }
        break;
    }

    case AgnessState::StandUp: {
        mAgnessStandUpTimer += deltaTime;

        ModelSettings s = agness->getInstanceSettings();
        float standUpDuration = agness->getAnimationEndTime(4);
        if (standUpDuration <= 0.01f) standUpDuration = 1.0f;

        s.msPlayAnimation = false;
        s.msAnimTimePosition = std::min(mAgnessStandUpTimer, standUpDuration - 0.01f);
        agness->setInstanceSettings(s);
        agness->checkForUpdates();

        if (mAgnessStandUpTimer >= standUpDuration) {
                        ModelSettings idle = agness->getInstanceSettings();
            idle.msAnimClip = 1;
            idle.msCrossBlendDestAnimClip = 1;
            idle.msBlendingMode = blendMode::crossfade;
            idle.msAnimCrossBlendFactor = 1.0f;
            idle.msAnimTimePosition = 0.0f;
            idle.msPlayAnimation = true;             agness->setInstanceSettings(idle);
            agness->checkForUpdates();

            mAgnessState = AgnessState::Idle;
        }
        break;
    }

    case AgnessState::Idle:
                break;
    }
}

bool OGLRenderer::init(unsigned int width, unsigned int height) {
  
  std::srand(static_cast<int>(time(NULL)));

  
  mRenderData.rdWidth = width;
  mRenderData.rdHeight = height;

  
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    Logger::log(1, "%s error: failed to initialize GLAD\n", __FUNCTION__);
    return false;
  }

  mCollisionConfiguration = new btDefaultCollisionConfiguration();
  mDispatcher = new btCollisionDispatcher(mCollisionConfiguration);
  mOverlappingPairCache = new btDbvtBroadphase();   mSolver = new btSequentialImpulseConstraintSolver();
  mDynamicsWorld = new btDiscreteDynamicsWorld(mDispatcher, mOverlappingPairCache, mSolver, mCollisionConfiguration);

  mDynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
  Logger::log(1, "Bullet Physics успешно инициализирован!\n");

  if (!GLAD_GL_VERSION_4_6) {
    Logger::log(1, "%s error: failed to get at least OpenGL 4.6\n", __FUNCTION__);
    return false;
  }
  /*
  if (glDebugMessageCallback) {
      glEnable(GL_DEBUG_OUTPUT);
      glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);       glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity,
          GLsizei length, const GLchar* message, const void* userParam) {
              fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
                  (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
                  type, severity, message);
          }, nullptr);
  }
  */
  GLint majorVersion, minorVersion;
  glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
  glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
  Logger::log(1, "%s: OpenGL %d.%d initializeed\n", __FUNCTION__, majorVersion, minorVersion);

  if (!mFramebuffer.init(width, height)) {
      Logger::log(1, "%s error: could not init Framebuffer\n", __FUNCTION__);
      return false;
  }

      if (!mMSFramebuffer.init(width, height, 4)) {
      Logger::log(1, "%s error: could not init MSFramebuffer\n", __FUNCTION__);
      return false;
  }
  Logger::log(1, "%s: framebuffer succesfully initialized\n", __FUNCTION__);

  if (!mBlurFBO[0].init(width, height) || !mBlurFBO[1].init(width, height)) {
      Logger::log(1, "%s error: could not init Bloom blur FBOs\n", __FUNCTION__);
      return false;
  }

    glGenVertexArrays(1, &mBloomVAO);

  Logger::log(1, "%s: bloom FBOs initialized\n", __FUNCTION__);
  mVertexBuffer.init();
  Logger::log(1, "%s: vertex buffer successfully created\n", __FUNCTION__);

  size_t uniformMatrixBufferSize = 2 * sizeof(glm::mat4);
  mUniformBuffer.init(uniformMatrixBufferSize);
  Logger::log(1, "%s: matrix uniform buffer (size %i bytes) successfully created\n", __FUNCTION__, uniformMatrixBufferSize);

  if (!mLineShader.loadShaders("shader/line.vert", "shader/line.frag")) {
      Logger::log(1, "%s: line shader loading failed\n", __FUNCTION__);
      return false;
  }
  if (!gAudio.init()) {
      Logger::log(1, "%s: line shader loading failed\n", __FUNCTION__);
  }

  if (!mBloomBlurShader.loadShaders("shader/Bloom_Blur.vert", "shader/Bloom_Blur.frag")) {
      Logger::log(1, "%s: mBlurShader loading failed\n", __FUNCTION__);
      return false;
  }
  if (!mBloomCompositeShader.loadShaders("shader/Bloom_Blur.vert", "shader/Bloom_Composite.frag")) {
      Logger::log(1, "%s: mCompositeShader loading failed\n", __FUNCTION__);
      return false;
  }

  if (!mDeathCaptureFBO.init(width, height)) {
      Logger::log(1, "%s error: could not init Death Screen capture FBO\n", __FUNCTION__);
      return false;
  }

  if (!mDeathGlowShader.loadShaders("shader/Bloom_Blur.vert", "shader/DeathScreen_Glow.frag")) {
      Logger::log(1, "%s: mDeathGlowShader loading failed\n", __FUNCTION__);
      return false;
  }

  if (!mDeathGlowCompositeShader.loadShaders("shader/Bloom_Blur.vert", "shader/DeathScreen_GlowComposite.frag")) {
      Logger::log(1, "%s: mDeathGlowCompositeShader loading failed\n", __FUNCTION__);
      return false;
  }

  if (!mGltfGPUShaderToy.loadShaders("shader/gltf_gpuToy.vert", "shader/gltf_gpuToy.frag")) {
      Logger::log(1, "%s: line shader loading failed\n", __FUNCTION__);
      return false;
  }

  if (!mGltfGPUShaderToyChannelPack.loadShaders("shader/gltf_gpuToyChannel.vert", "shader/gltf_gpuToyChannel.frag")) {
      Logger::log(1, "%s: line shader loading failed\n", __FUNCTION__);
      return false;
  }
  if (!mGltfGPUDualQuatChannelShader.loadShaders("shader/gltf_gpu_dquat.vert", "shader/gltf_gpuToyChannel.frag")) {
      Logger::log(1, "%s: advanced DQ shader loading failed\n", __FUNCTION__);
      return false;
  }

  if (!mStaticShader.loadShaders("shader/gltf.vert", "shader/gltf.frag")) {
      Logger::log(1, "%s: static shader loading failed\n", __FUNCTION__);
      return false;
  }

  if (!mGltfGPUShader.loadShaders("shader/gltf_gpu.vert", "shader/gltf_gpu.frag")) {
      Logger::log(1, "%s: gltTF GPU shader loading failed\n", __FUNCTION__);
      return false;
  }
  if (!mGltfGPUShader.getUniformLocation("aModelStride")) {
      Logger::log(1, "%s: failed to get model stride uniform for gltTF GPU shader\n",
          __FUNCTION__);
      return false;
  }

  if (!mGltfGPUDualQuatShader.loadShaders("shader/gltf_gpu_dquat.vert",
      "shader/gltf_gpu_dquat.frag")) {
      Logger::log(1, "%s: glTF GPU dual quat shader loading failed\n", __FUNCTION__);
      return false;
  }
  if (!mGltfGPUDualQuatShader.getUniformLocation("aModelStride")) {
      Logger::log(1, "%s: failed to get model stride uniform for gltTF GPU dual quat shader\n",
          __FUNCTION__);
      return false;
  }
  Logger::log(1, "%s: shaders succesfully loaded\n", __FUNCTION__);

  mUserInterface.init(mRenderData);
  Logger::log(1, "%s: user interface initialized\n", __FUNCTION__);

  
  glDisable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glLineWidth(3.0);
  
  if (!mSkyBox.init()) {
      Logger::log(1, "%s error: could not init skybox\n", __FUNCTION__);
      return false;
  }
  if (!mSkyBox.loadCrossCubemap("textures/SkyBox.png")) {
      Logger::log(1, "%s error: could not load skybox cubemap\n", __FUNCTION__);
      return false;
  }
  Logger::log(1, "%s: skybox successfully loaded\n", __FUNCTION__);

  if (!mSkyBox.generateIBLMaps()) {
      Logger::log(1, "%s error: could not generate IBL maps\n", __FUNCTION__);
      return false;
  }
  mSkyBox.addParallaxLayer(1, "textures/sky_mountainss.png",
      0.12f, 0.0f, { 0,1 }, 0.9f,
      0.1f);    {
      Logger::log(1, "%s warning: could not load mountains layer\n", __FUNCTION__);
        }

  if (!mSkyBox.addParallaxLayer(2, "textures/sky_cloudss.png",
      0.35f,          0.0004f, glm::vec2(1.0f, 0.5f)))    {
      Logger::log(1, "%s warning: could not load clouds layer\n", __FUNCTION__);
  }
  Logger::log(1, "%s: IBL maps successfully generated\n", __FUNCTION__);
  mGltfModels.resize(30);
  
  mNJMenuModel = std::make_shared<GltfModel>();
 
  std::string modelFilenameNJ = "assets/AngeBr.gltf";
  std::string modelTexFilenameNJ = "textures/Ange5.png";
  std::string modelTexFilenameNJnormal = "textures/AngeNormal2.png";
  std::string modelTexFilenameNJJ = "textures/angeCh2.png";
  std::string modelTexFilenameNJJJ = "textures/AngeCoat2.png";
  std::string modelTexFilenameNJJJJ = "textures/AngeEmit2.png";
  
  if (!mNJMenuModel->loadModel(mRenderData, modelFilenameNJ, modelTexFilenameNJ, modelTexFilenameNJnormal,"","", modelTexFilenameNJJJJ,modelTexFilenameNJJ
  , modelTexFilenameNJJJ)) {
      Logger::log(1, "%s error: could not load NJ menu model\n", __FUNCTION__);
      return false;
  } 
  mMenuController = std::make_unique<MenuController>();
  if (!mMenuController->init(mNJMenuModel)) {
      Logger::log(1, "%s error: MenuController init failed\n", __FUNCTION__);
      return false;
  }

  Logger::log(1, "%s: Main menu initialized\n", __FUNCTION__);
  
     
  
  mGltfModels.at(0) = std::make_shared<GltfModel>();
  std::string modelFilename = "assets/StonehengeAug.gltf";
  std::string modelTexFilename = "textures/StoneBase.png";
  std::string modelTexFilenameM = "textures/StoneMetal.png";
  std::string modelTexFilenameR = "textures/StoneRough.png";
  std::string modelTexFilenameN = "textures/StoneNormal.png";

  /* 11.04.26 нужно добавить сшиватель для roughness and metallic а то там только одно перемнная на двоих ну и шейдер доделать
  if (!mGltfModels.at(0)->loadModel(mRenderData, modelFilename, modelTexFilename, normalTex, roughTex)) {
      Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename.c_str());
      return false;
  }
  */
  if (!mGltfModels.at(0)->loadModel(mRenderData, modelFilename, modelTexFilename, modelTexFilenameN, modelTexFilenameM, modelTexFilenameR, "", "", "" )) {
      Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename.c_str());
      return false;
  }
 
  /*
  mGltfModels.at(1) = std::make_shared<GltfModel>();
  std::string modelFilename2 = "assets/enty6Clear.gltf";
  std::string modelTexFilename2 = "textures/enty4kClear.png";
  if (!mGltfModels.at(1)->loadModel(mRenderData, modelFilename2, modelTexFilename2)) {
    Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename2.c_str());
    return false;
  }
  */

  mGltfModels.at(1) = std::make_shared<GltfModel>();
  std::string modelFilename2 = "assets/EntyBrMix.gltf";
    std::string modelTexFilename2 = "textures/EntyAug.png";
  std::string modelTexFilename2N = "textures/EntyNormal.png";
  std::string modelTexFilename2M = "textures/EntyMetal.png";
  std::string modelTexFilename2R = "textures/EntyRough.png";
  if (!mGltfModels.at(1)->loadModel(mRenderData, modelFilename2, modelTexFilename2, modelTexFilename2N, modelTexFilename2M)) {
      Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename2.c_str());
      return false;
  }

  mGltfModels.at(2) = std::make_shared<GltfModel>();
  std::string modelFilename3 = "assets/Temple of Artemis.gltf";
  std::string modelTexFilename3 = "textures/Temple.png";
 
  if (!mGltfModels.at(2)->loadModel(mRenderData, modelFilename3, modelTexFilename3)) {
      Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename3.c_str());
      return false;
  }

  mGltfModels.at(3) = std::make_shared<GltfModel>();
  std::string modelFilename4 = "assets/vorota ishtar3.gltf";
  std::string modelTexFilename4 = "textures/VrataBase2.png";
  std::string modelTexFilename4N = "textures/VrataNormal2.png";
  std::string modelTexFilename4M = "textures/VrataMetal2.png";
  std::string modelTexFilename4R = "textures/VrataRough2.png";

  if (!mGltfModels.at(3)->loadModel(mRenderData, modelFilename4, modelTexFilename4, modelTexFilename4N, modelTexFilename4M, modelTexFilename4R)) {
      Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename4.c_str());
      return false;
  }

  mGltfModels.at(4) = std::make_shared<GltfModel>();
  std::string modelFilename5 = "assets/floor vorot ishtar2.gltf";
  std::string modelTexFilename5 = "textures/floorVrat16k.png";
  std::string modelTexFilename5N = "textures/floorVratNormal.png";
  std::string modelTexFilename5M = "textures/floorM.png";
  std::string modelTexFilename5R = "textures/floorR.png";

  if (!mGltfModels.at(4)->loadModel(mRenderData, modelFilename5, modelTexFilename5, modelTexFilename5N, modelTexFilename5M, modelTexFilename5R)) {
      Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename5.c_str());
      return false;
  }
 
  mGltfModels.at(5) = std::make_shared<GltfModel>();
  std::string modelFilename6 = "assets/kanalMay.gltf";
  std::string modelTexFilename6 = "textures/StockBase.png";
  if (!mGltfModels.at(5)->loadModel(mRenderData, modelFilename6, modelTexFilename6)) {
      Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename6.c_str());
      return false;
  }

  mGltfModels.at(6) = std::make_shared<GltfModel>();
  std::string modelFilename7 = "assets/gates 2.3.gltf";
  std::string modelTexFilename7 = "textures/GatesBase.png";
  std::string modelTexFilename7N = "textures/GatesNormal.png";
  std::string modelTexFilename7M = "textures/GatesMetal.png";
  std::string modelTexFilename7R = "textures/GatesRough.png";
  if (!mGltfModels.at(6)->loadModel(mRenderData, modelFilename7, modelTexFilename7, modelTexFilename7N, modelTexFilename7M, modelTexFilename7R)) {
      Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename7.c_str());
      return false;
  }

  mGltfModels.at(7) = std::make_shared<GltfModel>();
  std::string modelFilename8 = "assets/Arena.gltf";
  std::string modelTexFilename8 = "textures/Arena.png";
  if (!mGltfModels.at(7)->loadModel(mRenderData, modelFilename8, modelTexFilename8, "textures/ArenaNormal.png",
      "textures/ArenaMetal.png",
      "textures/ArenaRough.png",
      "textures/ArenaEmit.png")) {
      Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename8.c_str());
      return false;
  }
  mGltfModels.at(8) = std::make_shared<GltfModel>();
  std::string modelFilename9 = "assets/Symbiote.gltf";
  std::string modelTexFilename9 = "textures/Symbiote.png";
  if (!mGltfModels.at(8)->loadModel(mRenderData, modelFilename9, modelTexFilename9)) {
      Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename9.c_str());
      return false;
  }
  mGltfModels.at(9) = std::make_shared<GltfModel>();
  std::string modelFilename10 = "assets/MainMenuBox.gltf";
  std::string modelTexFilename10 = "textures/MainMenuBox.png";
  if (!mGltfModels.at(9)->loadModel(mRenderData, modelFilename10, modelTexFilename10)) {
      Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename10.c_str());
      return false;
  }
  mGltfModels.at(10) = std::make_shared<GltfModel>();
  std::string modelFilename11 = "assets/bismark.gltf";
  std::string modelTexFilename11 = "textures/Bismark.png";
  std::string modelTexFilename11N = "textures/BismarkNormal.png";
  std::string modelTexFilename11E = "textures/BismarkEmit.png";
  std::string modelTexFilename11Ch = "textures/BismarkCh.png";
  std::string modelTexFilename11Coat = "textures/BismarkNormal.png";
  if (!mGltfModels.at(10)->loadModel(mRenderData, modelFilename11, modelTexFilename11, 
      modelTexFilename11N,
      "",
      "",
      modelTexFilename11E,
      modelTexFilename11Ch,
      modelTexFilename11Coat)) {
      Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename11.c_str());
      return false;
  }
  mGltfModels.at(11) = std::make_shared<GltfModel>();
  std::string modelFilename12 = "assets/conway.gltf";
  std::string modelTexFilename12 = "textures/Conway.png";
  if (!mGltfModels.at(11)->loadModel(mRenderData, modelFilename12, modelTexFilename12)) {
      Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename11.c_str());
      return false;
  }

  mGltfModels.at(12) = std::make_shared<GltfModel>();
  std::string modelFilename13 = "assets/StoneAugPlants.gltf";
  std::string modelTexFilename13 = "textures/StonePlatns.png";
  if (!mGltfModels.at(12)->loadModel(mRenderData, modelFilename13, modelTexFilename13)) {
      Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename13.c_str());
      return false;
  }

  mGltfModels.at(13) = std::make_shared<GltfModel>();
  std::string modelFilename14 = "assets/Agness.gltf";
  std::string modelTexFilename14 = "textures/Agness.png";
  std::string modelTexFilename14N = "textures/AgnessNormal.png";
  std::string modelTexFilename14M = "textures/AgnessMetal.png";
  std::string modelTexFilename14R = "textures/AgnessRough.png";
  if (!mGltfModels.at(13)->loadModel(mRenderData, modelFilename14, modelTexFilename14, modelTexFilename14N, modelTexFilename14M, modelTexFilename14R)) {
      Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename14.c_str());
      return false;
  }

  mGltfModels.at(14) = std::make_shared<GltfModel>();
  std::string modelFilename15 = "assets/boxG.gltf";
  if (!mGltfModels.at(14)->loadModel(mRenderData, modelFilename15, modelTexFilename14)) {
      Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename15.c_str());
      return false;
  }
      /*
  mGltfModels.at(1) = std::make_shared<GltfModel>();
  modelTexFilename = "textures/FigureBase Color - 4096px.png";
  if (!mGltfModels.at(1)->loadModel(mRenderData, modelFilename, modelTexFilename)) {
    Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename.c_str());
    return false;
  }
  mGltfModels.at(1)->uploadVertexBuffers();
  mGltfModels.at(1)->uploadIndexBuffer();

  mGltfModels.at(2) = std::make_shared<GltfModel>();
  modelFilename = "assets/dq.gltf";
  modelTexFilename = "textures/FigureBase Color - 4096px.png";
  if (!mGltfModels.at(2)->loadModel(mRenderData, modelFilename, modelTexFilename)) {
    Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename.c_str());
    return false;
  }
  mGltfModels.at(2)->uploadVertexBuffers();
  mGltfModels.at(2)->uploadIndexBuffer();
  */
  Logger::log(1, "%s: glTF model '%s' succesfully loaded\n", __FUNCTION__, modelFilename.c_str());

  int numTriangles = 0;

  
  /*
  for (int i = 0; i < 5; ++i) {
    int xPos = std::rand() % 40 - 20;
    int zPos = std::rand() % 40 - 20;
    int modelNo = std::rand() % 2;
    mGltfInstances.emplace_back(std::make_shared<GltfInstance>(mGltfModels.at(modelNo), glm::vec2(static_cast<float>(xPos),
      static_cast<float>(zPos)), true));
    numTriangles += mGltfModels.at(modelNo)->getTriangleCount();
  }
    
  for (int i = 0; i < 10; ++i) {
    int xPos = std::rand() % 50 - 25;
    int zPos = std::rand() % 20 - 50;
    mGltfInstances.emplace_back(std::make_shared<GltfInstance>(mGltfModels.at(2), glm::vec2(static_cast<float>(xPos),
      static_cast<float>(zPos)), true));
    numTriangles += mGltfModels.at(2)->getTriangleCount();
  }
  */
    glm::vec3 defaultRot = glm::vec3(0.0);

  int xPos = 77;
  int yPos = 0;
  int zPos = 580;
  mGltfInstances.emplace_back(std::make_shared<GltfInstance>(mGltfModels.at(0), glm::vec3(static_cast<float>(xPos), static_cast<float>(yPos),
      static_cast<float>(zPos)), defaultRot));
  numTriangles += static_cast<unsigned int>(mGltfModels.at(0)->getTriangleCount());

  int xPos2 = 0;
  int yPos2 = 3;
  int zPos2 = 0;
  mGltfInstances.emplace_back(std::make_shared<GltfInstance>(mGltfModels.at(1), glm::vec3(static_cast<float>(xPos2), static_cast<float>(yPos2),
      static_cast<float>(zPos2)), defaultRot));
  numTriangles += static_cast<unsigned int>(mGltfModels.at(1)->getTriangleCount());

  int xPos3 = 50;
  double yPos3 = 0.1;
  int zPos3 = 419;
  glm::vec3 CustomRot = glm::vec3(0.0f, 90.0f, 0.0f);
  mGltfInstances.emplace_back(std::make_shared<GltfInstance>(mGltfModels.at(2), glm::vec3(static_cast<float>(xPos3), static_cast<float>(yPos3),
      static_cast<float>(zPos3)), CustomRot));
  numTriangles += static_cast<unsigned int>(mGltfModels.at(2)->getTriangleCount());
  
  int xPos4 = 0;
  int yPos4 = 0;
  int zPos4 = 160;
  
  mGltfInstances.emplace_back(std::make_shared<GltfInstance>(mGltfModels.at(3), glm::vec3(static_cast<float>(xPos4), static_cast<float>(yPos4),
      static_cast<float>(zPos4)), defaultRot));
  numTriangles += static_cast<unsigned int>(mGltfModels.at(3)->getTriangleCount());

  int xPos5 = 0;
  int yPos5 = 0;
  int zPos5 = 180;
  mGltfInstances.emplace_back(std::make_shared<GltfInstance>(mGltfModels.at(4), glm::vec3(static_cast<float>(xPos5), static_cast<float>(yPos5),
      static_cast<float>(zPos5)), defaultRot));
  numTriangles += static_cast<unsigned int>(mGltfModels.at(4)->getTriangleCount());

  int xPos6 = -166;
  int yPos6 = 1;
  int zPos6 = 261;
  glm::vec3 CustomRot2 = glm::vec3(0.0f, -88.0f, 0.0f);
  mGltfInstances.emplace_back(std::make_shared<GltfInstance>(mGltfModels.at(5), glm::vec3(static_cast<float>(xPos6), static_cast<float>(yPos6),
      static_cast<float>(zPos6)), CustomRot2));
  numTriangles += static_cast<unsigned int>(mGltfModels.at(5)->getTriangleCount());

  int xPos7 = 53;
  int yPos7 = 0;
  int zPos7 = 522;
  mGltfInstances.emplace_back(std::make_shared<GltfInstance>(mGltfModels.at(6), glm::vec3(static_cast<float>(xPos7), static_cast<float>(yPos7),
      static_cast<float>(zPos7)), defaultRot));
  numTriangles += static_cast<unsigned int>(mGltfModels.at(6)->getTriangleCount());

  int xPos8 = 394;
  int yPos8 = 0;
  int zPos8 = 322;
  glm::vec3 CustomRot3 = glm::vec3(0.0f, 90.0f, 0.0f);
  mGltfInstances.emplace_back(std::make_shared<GltfInstance>(mGltfModels.at(7), glm::vec3(static_cast<float>(xPos8), static_cast<float>(yPos8),
      static_cast<float>(zPos8)), CustomRot3));
  numTriangles += static_cast<unsigned int>(mGltfModels.at(7)->getTriangleCount());

  mGltfInstances.emplace_back(std::make_shared<GltfInstance>(mGltfModels.at(12), glm::vec3(static_cast<float>(xPos), static_cast<float>(yPos),
      static_cast<float>(zPos)), defaultRot));
  numTriangles += static_cast<unsigned int>(mGltfModels.at(12)->getTriangleCount());

  int xPos9 = 81;
  int yPos9 = 2;
  int zPos9 = 615;
  mGltfInstances.emplace_back(std::make_shared<GltfInstance>(mGltfModels.at(13), glm::vec3(static_cast<float>(xPos9), static_cast<float>(yPos9),
      static_cast<float>(zPos9)), defaultRot));
  numTriangles += static_cast<unsigned int>(mGltfModels.at(13)->getTriangleCount());

  int xPos10 = 53;
  int yPos10 = -100;
  int zPos10 = 70;
  mGltfInstances.emplace_back(std::make_shared<GltfInstance>(mGltfModels.at(14), glm::vec3(static_cast<float>(xPos10), static_cast<float>(yPos10),
      static_cast<float>(zPos10)), defaultRot));
  numTriangles += static_cast<unsigned int>(mGltfModels.at(14)->getTriangleCount());

  addEnemy(mGltfModels.at(8), -17, 1, 147);
  addEnemy(mGltfModels.at(8), 15, 1, 148);
  addEnemy(mGltfModels.at(8), 13, 1, 166);

  addEnemy(mGltfModels.at(8), -10, 1, 165);
  addEnemy(mGltfModels.at(8), -3, 1, 65);
  addEnemy(mGltfModels.at(8), -158, 2, 259);

  addEnemy(mGltfModels.at(8), -206, 2, 268);
  addEnemy(mGltfModels.at(8), -207, 2, 256);
  addEnemy(mGltfModels.at(8), -205, 2, 229);

  addEnemy(mGltfModels.at(8), -250, 10, 224);
  addEnemy(mGltfModels.at(8), -311, 10, 252);
  addEnemy(mGltfModels.at(8), -220, 10, 313);

  addBossBismark(mGltfModels.at(10), 49, 8, 408);
  mBismarkBossPtr = mEnemies.back().get();   addBossConway(mGltfModels.at(11), 389, 1, 323);
  addPhysicsToStonehenge(mGltfInstances.at(0), mDynamicsWorld, glm::vec3(77, 0, 580),defaultRot);

  addPhysicsToEnty(mGltfInstances.at(1), mDynamicsWorld, glm::vec3(0, 0, 0));   {
      ModelSettings entySettings = mGltfInstances.at(1)->getInstanceSettings();
      entySettings.msAnimClip = EntyAnim::Idle; 
      entySettings.msPlayAnimation = true;      
      mGltfInstances.at(1)->setInstanceSettings(entySettings);
      Logger::log(1, "Enty anim count: %zu\n", mGltfModels.at(1)->getAnimClips().size());
  }

  addPhysicsToEnty(mGltfInstances.at(9), mDynamicsWorld, glm::vec3(81, 2, 615));
  {
      ModelSettings AgnessSettings = mGltfInstances.at(9)->getInstanceSettings();
      AgnessSettings.msAnimClip = 3;
      AgnessSettings.msCrossBlendDestAnimClip = 3;
      AgnessSettings.msBlendingMode = blendMode::crossfade;
      AgnessSettings.msAnimCrossBlendFactor = 1.0f;
      AgnessSettings.msAnimTimePosition = 0.0f;
      AgnessSettings.msPlayAnimation = true;
      mGltfInstances.at(9)->setInstanceSettings(AgnessSettings);
      mGltfInstances.at(9)->checkForUpdates();

                        Logger::log(1, "Agness anim count: %zu (запрошенный клип: %d)\n",
          mGltfModels.at(13)->getAnimClips().size(), AgnessSettings.msAnimClip);
  }
  {
      auto root = mGltfInstances.at(1)->getRootNode();

                        
      auto leftNode = findNodeByName(root, "Breast.L");
      auto rightNode = findNodeByName(root, "Breast.R");

      if (leftNode || rightNode) {
          Logger::log(1, "%s  Breast.L/Breast.R found on Enty skeleton\n", __FUNCTION__);
      }
      if (!leftNode || !rightNode) {
          Logger::log(1, "%s warning: Breast.L/Breast.R not found on Enty skeleton\n", __FUNCTION__);
      }

      initBreastJiggle(mLeftBreastJiggle, leftNode,40.0f,4.0f,18.0f,1.5f);
      initBreastJiggle(mRightBreastJiggle, rightNode, 40.0f, 4.0f, 18.0f, 1.5f);
  }
  addPhysicsToStonehenge(mGltfInstances.at(3), mDynamicsWorld, glm::vec3(0, 0, 160), defaultRot);
  addPhysicsToStonehenge(mGltfInstances.at(4), mDynamicsWorld, glm::vec3(0, 0, 180),defaultRot);
  addPhysicsToStonehenge(mGltfInstances.at(2), mDynamicsWorld, glm::vec3(50, 0.1, 419),CustomRot);
  addPhysicsToStonehenge(mGltfInstances.at(5), mDynamicsWorld, glm::vec3(-166, 1, 261), CustomRot2);
  addPhysicsToStonehenge(mGltfInstances.at(6), mDynamicsWorld, glm::vec3(53, 0, 522), defaultRot);
  addPhysicsToStonehenge(mGltfInstances.at(7), mDynamicsWorld, glm::vec3(394, 0, 322), CustomRot3);
    addPhysicsToStonehenge(mGltfInstances.at(10), mDynamicsWorld, glm::vec3(53, 1, 513), defaultRot);
  
  mRenderData.rdTriangleCount = numTriangles;

  mRenderData.rdNumberOfInstances = mGltfInstances.size();

  size_t modelJointMatrixBufferSize = 0;
  size_t modelJointDualQuatBufferSize = 0;
  int jointMatrixSize = 0;
  int jointQuatSize = 0;

  for (const auto &instance : mGltfInstances) {
    printf("[DEBUG] Instance Joints: %d\n", instance->getJointMatrixSize());
    jointMatrixSize += instance->getJointMatrixSize();
    modelJointMatrixBufferSize += instance->getJointMatrixSize() * sizeof(glm::mat4);

    jointQuatSize += instance->getJointDualQuatsSize();
    modelJointDualQuatBufferSize += instance->getJointDualQuatsSize() *
      sizeof(glm::mat2x4);
  }
    printf("\n--- [SSBO DEBUG] ---\n");
  printf("Total Joint Matrices: %d\n", jointMatrixSize);
  printf("Required Matrix Buffer Size: %llu bytes\n", (unsigned long long)modelJointMatrixBufferSize);
  printf("Required DualQuat Buffer Size: %llu bytes\n", (unsigned long long)modelJointDualQuatBufferSize);

    GLint maxSize;
  glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxSize);
  printf("Your GPU Max SSBO Size: %d bytes\n", maxSize);

  if (modelJointMatrixBufferSize == 0) {
      printf("!!! WARNING: Matrix Buffer Size is 0! This will cause Error 502.\n");
  }

  if (modelJointMatrixBufferSize > (size_t)maxSize) {
      printf("!!! ERROR: Buffer size exceeds GPU limits!\n");
  }

  if (modelJointMatrixBufferSize == 0) modelJointMatrixBufferSize = sizeof(glm::mat4);
  if (modelJointDualQuatBufferSize == 0) modelJointDualQuatBufferSize = sizeof(glm::mat2x4);

    mGltfShaderStorageBuffer.init(modelJointMatrixBufferSize);
  Logger::log(1, "%s: glTF joint matrix shader storage buffer (size %i bytes) successfully created\n", __FUNCTION__, modelJointMatrixBufferSize);
  
  mGltfDualQuatSSBuffer.init(modelJointDualQuatBufferSize);
  Logger::log(1, "%s: glTF joint dual quaternions shader storage buffer (size %i bytes) successfully created\n", __FUNCTION__, modelJointDualQuatBufferSize);

  
  Logger::log(1, "%s: line mesh storage initialized\n", __FUNCTION__);

  mFrameTimer.start();
  Logger::log(1, "%s: Starting to load instances...\n", __FUNCTION__);
  return true;
}

void OGLRenderer::setSize(unsigned int width, unsigned int height) {
   
    if (width == 0 || height == 0) {
        return;
    }

    mRenderData.rdWidth = width;
    mRenderData.rdHeight = height;

    mFramebuffer.resize(width, height);
    mMSFramebuffer.resize(width, height);
    mBlurFBO[0].resize(width, height);
    mBlurFBO[1].resize(width, height);
    mDeathCaptureFBO.resize(width, height);
    glViewport(0, 0, width, height);

    Logger::log(1, "%s: resized window to %dx%d\n", __FUNCTION__, width, height);
}

void OGLRenderer::uploadData(OGLMesh vertexData) {
  mVertexBuffer.uploadData(vertexData);
}

void OGLRenderer::GltfGPUShaderToy(unsigned int matrixPos)
{
    if (mGltfGPUShaderToy.getUniformLocation("u_SunDirection"))   mGltfGPUShaderToy.setUniformValue(mRenderData.rdSunDirection);
    if (mGltfGPUShaderToy.getUniformLocation("u_SunColor"))       mGltfGPUShaderToy.setUniformValue(mRenderData.rdSunColor);
    if (mGltfGPUShaderToy.getUniformLocation("u_SunIntensity"))   mGltfGPUShaderToy.setUniformValue(mRenderData.rdSunIntensity);
    if (mGltfGPUShaderToy.getUniformLocation("u_LightPos"))       mGltfGPUShaderToy.setUniformValue(mRenderData.rdLightPos);
    if (mGltfGPUShaderToy.getUniformLocation("u_PointColor"))     mGltfGPUShaderToy.setUniformValue(mRenderData.rdPointColor);
    if (mGltfGPUShaderToy.getUniformLocation("u_PointIntensity")) mGltfGPUShaderToy.setUniformValue(mRenderData.rdPointIntensity);
    if (mGltfGPUShaderToy.getUniformLocation("u_BaseColorMap"))   mGltfGPUShaderToy.setUniformValue(0);
    if (mGltfGPUShaderToy.getUniformLocation("u_NormalMap"))      mGltfGPUShaderToy.setUniformValue(1);
    if (mGltfGPUShaderToy.getUniformLocation("u_MetallicMap"))    mGltfGPUShaderToy.setUniformValue(2);
    if (mGltfGPUShaderToy.getUniformLocation("u_RoughnessMap"))   mGltfGPUShaderToy.setUniformValue(3);
    if (mGltfGPUShaderToy.getUniformLocation("u_IrradianceMap"))  mGltfGPUShaderToy.setUniformValue(5);
    if (mGltfGPUShaderToy.getUniformLocation("u_PrefilterMap"))   mGltfGPUShaderToy.setUniformValue(6);
    if (mGltfGPUShaderToy.getUniformLocation("u_BRDFLUT"))        mGltfGPUShaderToy.setUniformValue(7);
    if (mGltfGPUShaderToy.getUniformLocation("aModelStride"))     mGltfGPUShaderToy.setUniformValue((int)matrixPos);
}

void OGLRenderer::GltfGPUShaderToyChannelPack(unsigned int matrixPos, const std::shared_ptr<GltfInstance>& instance)
{
    mGltfGPUShaderToyChannelPack.use();
    if (mGltfGPUShaderToyChannelPack.getUniformLocation("u_SunDirection"))    mGltfGPUShaderToyChannelPack.setUniformValue(mRenderData.rdSunDirection);
    if (mGltfGPUShaderToyChannelPack.getUniformLocation("u_SunColor"))        mGltfGPUShaderToyChannelPack.setUniformValue(mRenderData.rdSunColor);
    if (mGltfGPUShaderToyChannelPack.getUniformLocation("u_SunIntensity"))    mGltfGPUShaderToyChannelPack.setUniformValue(mRenderData.rdSunIntensity);
    if (mGltfGPUShaderToyChannelPack.getUniformLocation("u_LightPos"))        mGltfGPUShaderToyChannelPack.setUniformValue(mRenderData.rdLightPos);
    if (mGltfGPUShaderToyChannelPack.getUniformLocation("u_PointColor"))      mGltfGPUShaderToyChannelPack.setUniformValue(mRenderData.rdPointColor);
    if (mGltfGPUShaderToyChannelPack.getUniformLocation("u_PointIntensity"))  mGltfGPUShaderToyChannelPack.setUniformValue(mRenderData.rdPointIntensity);
    if (mGltfGPUShaderToyChannelPack.getUniformLocation("u_AlbedoMap"))       mGltfGPUShaderToyChannelPack.setUniformValue(0);
    if (mGltfGPUShaderToyChannelPack.getUniformLocation("u_NormalMap"))       mGltfGPUShaderToyChannelPack.setUniformValue(1);
    if (mGltfGPUShaderToyChannelPack.getUniformLocation("u_PackedMap"))       mGltfGPUShaderToyChannelPack.setUniformValue(2);
    if (mGltfGPUShaderToyChannelPack.getUniformLocation("u_CoatRoughMap"))    mGltfGPUShaderToyChannelPack.setUniformValue(3);
    if (mGltfGPUShaderToyChannelPack.getUniformLocation("u_EmitMap"))         mGltfGPUShaderToyChannelPack.setUniformValue(4);
    if (mGltfGPUShaderToyChannelPack.getUniformLocation("u_IrradianceMap"))   mGltfGPUShaderToyChannelPack.setUniformValue(5);
    if (mGltfGPUShaderToyChannelPack.getUniformLocation("u_PrefilterMap"))    mGltfGPUShaderToyChannelPack.setUniformValue(6);
    if (mGltfGPUShaderToyChannelPack.getUniformLocation("u_BRDFLUT"))         mGltfGPUShaderToyChannelPack.setUniformValue(7);
    if (mGltfGPUShaderToyChannelPack.getUniformLocation("u_HasNormalMap"))    mGltfGPUShaderToyChannelPack.setUniformValue(instance->getModel()->mHasNormalMap ? 1 : 0);
    if (mGltfGPUShaderToyChannelPack.getUniformLocation("u_HasPackedMap"))    mGltfGPUShaderToyChannelPack.setUniformValue(instance->getModel()->mHasPackedMap ? 1 : 0);
    if (mGltfGPUShaderToyChannelPack.getUniformLocation("u_HasCoatRoughMap")) mGltfGPUShaderToyChannelPack.setUniformValue(instance->getModel()->mHasCoatRoughMap ? 1 : 0);
    if (mGltfGPUShaderToyChannelPack.getUniformLocation("u_HasEmitMap"))      mGltfGPUShaderToyChannelPack.setUniformValue(instance->getModel()->mHasEmitMap ? 1 : 0);
        float emitIntensity = UserInterface::showEmit ? 3.0f : 0.0f;

    if (mGltfGPUShaderToyChannelPack.getUniformLocation("u_EmissionMultiplier"))
        mGltfGPUShaderToyChannelPack.setUniformValue(emitIntensity);
    if (mGltfGPUShaderToyChannelPack.getUniformLocation("aModelStride"))      mGltfGPUShaderToyChannelPack.setUniformValue((int)matrixPos);
}

void OGLRenderer::handleKeyEvents(int key, int scancode, int action, int mods) {
    if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        mShowMenu = !mShowMenu;
        Logger::log(1, "%s: Menu state toggled: %s\n", __FUNCTION__, mShowMenu ? "ON" : "OFF");
        if (mShowMenu) {
            glfwSetInputMode(mRenderData.rdWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else {
                                    glfwSetInputMode(mRenderData.rdWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

void OGLRenderer::handleMouseButtonEvents(int button, int action, int mods) {
  
  ImGuiIO& io = ImGui::GetIO();
  if (button >= 0 && button < ImGuiMouseButton_COUNT) {
    io.AddMouseButtonEvent(button, action == GLFW_PRESS);
  }

  
  if (io.WantCaptureMouse) {
    return;
  }
  
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
      mMouseLock = !mMouseLock;
    if (mMouseLock) {
      glfwSetInputMode(mRenderData.rdWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      
      if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(mRenderData.rdWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
      }
    } else {
      glfwSetInputMode(mRenderData.rdWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
  }
}

void OGLRenderer::handleMousePositionEvents(double xPos, double yPos) {
  
  ImGuiIO& io = ImGui::GetIO();
  io.AddMousePosEvent((float)xPos, (float)yPos);

  
  if (io.WantCaptureMouse) {
    return;
  }
  if (mShowMenu) {
      return;
  }
  
  int mouseMoveRelX = static_cast<int>(xPos) - mMouseXPos;
  int mouseMoveRelY = static_cast<int>(yPos) - mMouseYPos;

  if (mMouseLock) {
    mRenderData.rdViewAzimuth += mouseMoveRelX / 10.0;
    
    if (mRenderData.rdViewAzimuth < 0.0) {
      mRenderData.rdViewAzimuth += 360.0;
    }
    if (mRenderData.rdViewAzimuth >= 360.0) {
      mRenderData.rdViewAzimuth -= 360.0;
    }

    mRenderData.rdViewElevation -= mouseMoveRelY / 10.0;
    
    if (mRenderData.rdViewElevation > 89.0) {
      mRenderData.rdViewElevation = 89.0;
    }
    if (mRenderData.rdViewElevation < -89.0) {
      mRenderData.rdViewElevation = -89.0;
    }
  }

  
  mMouseXPos = static_cast<int>(xPos);
  mMouseYPos = static_cast<int>(yPos);
}

void OGLRenderer::addPhysicsToEnty(std::shared_ptr<GltfInstance> entyInstance, btDiscreteDynamicsWorld* dynamicsWorld, glm::vec3 spawnPos) {
            btCollisionShape* colShape = new btCapsuleShape(0.3f, 1.8f);

    btTransform startTransform;
    startTransform.setIdentity();
        startTransform.setOrigin(btVector3(spawnPos.x, spawnPos.y + 5.0f, spawnPos.z));

    btScalar mass(70.0f);
    btVector3 localInertia(0, 0, 0);
    colShape->calculateLocalInertia(mass, localInertia);

    btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
    btRigidBody* body = new btRigidBody(rbInfo);

    body->setAngularFactor(btVector3(0, 1, 0));

    dynamicsWorld->addRigidBody(body);
    entyInstance->setRigidBody(body);
}
void OGLRenderer::addPhysicsToEnemy(std::shared_ptr<GltfInstance> entyInstance, btDiscreteDynamicsWorld* dynamicsWorld, glm::vec3 spawnPos) {
            btCollisionShape* colShape = new btCapsuleShape(0.3f, 1.8f);

    btTransform startTransform;
    startTransform.setIdentity();
        startTransform.setOrigin(btVector3(spawnPos.x, spawnPos.y + 5.0f, spawnPos.z));

    btScalar mass(700.0f);
    btVector3 localInertia(0, 0, 0);
    colShape->calculateLocalInertia(mass, localInertia);

    btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
    btRigidBody* body = new btRigidBody(rbInfo);

    body->setAngularFactor(btVector3(0, 1, 0));

    dynamicsWorld->addRigidBody(body);
    entyInstance->setRigidBody(body);
}
void OGLRenderer::addEnemy(std::shared_ptr<GltfModel> model, int x, int y, int z) {
    glm::vec3 defaultRot = glm::vec3(0.0f);
    mGltfInstances.emplace_back(std::make_shared<GltfInstance>(model, glm::vec3(static_cast<float>(x), static_cast<float>(y),
        static_cast<float>(z)), defaultRot));

    auto& newInstance = mGltfInstances.back();

    addPhysicsToEnemy(newInstance, mDynamicsWorld, glm::vec3(x, y, z));
    auto enemyAgent = std::make_unique<Enemy>(newInstance, newInstance->getRigidBody());
    mEnemies.push_back(std::move(enemyAgent));
}
void OGLRenderer::addBossConway(std::shared_ptr<GltfModel> model, int x, int y, int z)
{
    glm::vec3 defaultRot = glm::vec3(0.0f);
    mGltfInstances.emplace_back(std::make_shared<GltfInstance>(model, glm::vec3(static_cast<float>(x), static_cast<float>(y),
        static_cast<float>(z)), defaultRot));
    auto& newInstance = mGltfInstances.back();
    addPhysicsToEnemy(newInstance, mDynamicsWorld, glm::vec3(x, y, z));
    auto bossAgent = std::make_unique<BossConway>(newInstance, newInstance->getRigidBody());
    mEnemies.push_back(std::move(bossAgent));
}
void OGLRenderer::addBossBismark(std::shared_ptr<GltfModel> model, int x, int y, int z)
{
    glm::vec3 defaultRot = glm::vec3(0.0f);
    mGltfInstances.emplace_back(std::make_shared<GltfInstance>(model, glm::vec3(static_cast<float>(x), static_cast<float>(y),
        static_cast<float>(z)), defaultRot));
    auto& newInstance = mGltfInstances.back();
    addPhysicsToEnemy(newInstance, mDynamicsWorld, glm::vec3(x, y, z));
    auto bossAgent = std::make_unique<BossBismark>(newInstance, newInstance->getRigidBody());
    mEnemies.push_back(std::move(bossAgent));
}
void OGLRenderer::addPhysicsToStonehenge(std::shared_ptr<GltfInstance> stonehengeInstance, btDiscreteDynamicsWorld* dynamicsWorld, glm::vec3 pos, glm::vec3 rot) {
        btCollisionShape* groundShape = stonehengeInstance->getModel()->getPhysicsShape();

    btTransform groundTransform;
    groundTransform.setIdentity();

            groundTransform.setOrigin(btVector3(pos.x, pos.y, pos.z));

                btQuaternion btRot;
    btRot.setEuler(
        glm::radians(rot.y),         glm::radians(rot.x),         glm::radians(rot.z)      );
    groundTransform.setRotation(btRot);

        btScalar mass(0.0f);
    btVector3 localInertia(0, 0, 0);

    btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
    btRigidBody* body = new btRigidBody(rbInfo);

        body->setFriction(0.8f);

    dynamicsWorld->addRigidBody(body);
    stonehengeInstance->setRigidBody(body);
}

void OGLRenderer::handleMovementKeys() {
  mRenderData.rdMoveForward = 0;
  if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_W) == GLFW_PRESS) {
    mRenderData.rdMoveForward += 1;
  }
  if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_S) == GLFW_PRESS) {
    mRenderData.rdMoveForward -= 1;
  }

  mRenderData.rdMoveRight = 0;
  if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_A) == GLFW_PRESS) {
    mRenderData.rdMoveRight -= 1;
  }
  if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_D) == GLFW_PRESS) {
    mRenderData.rdMoveRight += 1;
  }

  mRenderData.rdMoveUp = 0;
  if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_E) == GLFW_PRESS) {
    mRenderData.rdMoveUp += 1;
  }
  if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_Q) == GLFW_PRESS) {
    mRenderData.rdMoveUp -= 1;
  }

  
  if ((glfwGetKey(mRenderData.rdWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ||
      (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)) {
    mRenderData.rdMoveForward *= 4;
    mRenderData.rdMoveRight *= 4;
    mRenderData.rdMoveUp *= 4;
  }
}

void OGLRenderer::draw() {
                while (mRenderData.rdWidth == 0 || mRenderData.rdHeight == 0) {
        glfwGetFramebufferSize(mRenderData.rdWindow, &mRenderData.rdWidth, &mRenderData.rdHeight);
        glfwWaitEvents();
    }

    double tickTime = glfwGetTime();
    mRenderData.rdTickDiff = tickTime - mLastTickTime;
    float deltaTime = static_cast<float>(mRenderData.rdTickDiff);
    if (deltaTime > 0.1f) deltaTime = 0.1f;

    mRenderData.rdFrameTime = mFrameTimer.stop();
    mFrameTimer.start();

                switch (mFadeState) {
    case FadeState::FADE_OUT:
        mFadeAlpha += deltaTime / FADE_DURATION;
        if (mFadeAlpha >= 1.0f) {
            mFadeAlpha = 1.0f;
            mFadeState = FadeState::LOADING_WAIT;
            mLoadingTimer = 0.0f;
            mLoadingSpinAngle = 0.0f;
            mGameState = GameState::LOADING;
            Logger::log(1, "OGLRenderer: Screen black, entering LOADING state\n");
        }
        break;

    case FadeState::LOADING_WAIT:
        mLoadingTimer += deltaTime;
        mLoadingSpinAngle += deltaTime * 3.2f;
        if (mLoadingTimer >= LOADING_MIN_TIME) {
            mFadeState = FadeState::FADE_IN;
            mGameState = GameState::GAMEPLAY;
            Logger::log(1, "OGLRenderer: Loading complete, fading into GAMEPLAY\n");
        }
        break;

    case FadeState::FADE_IN:
        mFadeAlpha -= deltaTime / FADE_DURATION;
        if (mFadeAlpha <= 0.0f) {
            mFadeAlpha = 0.0f;
            mFadeState = FadeState::NONE;
        }
        break;

    case FadeState::NONE:
    default:
        break;
    }
    
    if (mRenderData.rdPlayerIsDead && !mShowDeathScreen && !mRespawnRequested) {
        mShowDeathScreen = true;
        mDeathFadeAlpha = 0.0f;
        mGameState = GameState::DEATH;                   mActiveBossEnemy = nullptr;
        mBossBarVisibleAlpha = 0.0f;
        mBossBarDisplayHealth = 0.0f;

      
        Logger::log(1, "OGLRenderer: Player died — entering DEATH state\n");
    }

    mRespawnGlowPulse += deltaTime;

    if (mShowDeathScreen && !mRespawnRequested) {
                mDeathFadeAlpha += deltaTime / DEATH_FADE_DURATION;
        if (mDeathFadeAlpha > DEATH_FADE_MAX) mDeathFadeAlpha = DEATH_FADE_MAX;
    }
    else if (mRespawnRequested) {
                mDeathFadeAlpha -= deltaTime / (DEATH_FADE_DURATION * 0.6f);
        if (mDeathFadeAlpha <= 0.0f) {
            mDeathFadeAlpha = 0.0f;
            mShowDeathScreen = false;
            mRespawnRequested = false;
            ResetPlayerAfterDeath();
        }
    }
                bool pendingFadeOut = false;

    if (mGameState == GameState::MAIN_MENU) {
        MenuAction action = mMenuController->update(
            deltaTime,
            mRenderData.rdMenuAnyButtonHovered
        );
        if (action == MenuAction::START_GAME && mFadeState == FadeState::NONE) {
            pendingFadeOut = true;
        }
        else if (action == MenuAction::CREDITS) {
            mRenderData.showCreditsWindow = true;
        }
        else if (action == MenuAction::QUIT) {
            glfwSetWindowShouldClose(mRenderData.rdWindow, GLFW_TRUE);
        }
    }
    else if (mGameState == GameState::LOADING) {
                gAudio.playMusic("assets/audio/ambient.MP3", true);
    }
    else if (mGameState == GameState::DEATH) {
                                if (mDynamicsWorld) mDynamicsWorld->stepSimulation(deltaTime, 10);
        for (auto& instance : mGltfInstances) instance->syncPhysics();
        updatePlayerCharacter(deltaTime);
                    }
    else {
                if (mDynamicsWorld) mDynamicsWorld->stepSimulation(deltaTime, 10);
        for (auto& instance : mGltfInstances) instance->syncPhysics();

        handleMovementKeys();
        updatePlayerCharacter(deltaTime);

        btTransform entyTrans;
        mGltfInstances.at(1)->getRigidBody()->getMotionState()->getWorldTransform(entyTrans);
        glm::vec3 entyPos(
            entyTrans.getOrigin().x(),
            entyTrans.getOrigin().y(),
            entyTrans.getOrigin().z()
        );
        for (auto& enemy : mEnemies) enemy->update(deltaTime, entyPos);
        updateAgness(deltaTime, entyPos, mRenderData);                                 if (mBismarkGateCollisionActive && mBismarkBossPtr && mBismarkBossPtr->isDead()) {
            btRigidBody* gateBody = mGltfInstances.at(10)->getRigidBody();
            if (gateBody && mDynamicsWorld) {
                mDynamicsWorld->removeRigidBody(gateBody);
            }
            mBismarkGateCollisionActive = false;
            Logger::log(1, "OGLRenderer: Bismark defeated — gate collision removed\n");
        }
                Enemy* foundBoss = nullptr;
        for (auto& enemy : mEnemies) {
            if (enemy->isBoss() && enemy->isEngaged() && !enemy->isDead()) {
                foundBoss = enemy.get();
                break;             }
        }

        if (foundBoss != mActiveBossEnemy) {
                                    if (mActiveBossEnemy && !foundBoss) {
                                gAudio.stopMusic();
                gAudio.playMusic("assets/audio/ambient.MP3", true);             }
            else if (!mActiveBossEnemy && foundBoss) {
                                gAudio.stopMusic();
                foundBoss->playmusic();
                                            }
            else if (mActiveBossEnemy && foundBoss && mActiveBossEnemy != foundBoss) {
                                gAudio.stopMusic();
                foundBoss->playmusic();
            }
            mActiveBossEnemy = foundBoss;
            if (mActiveBossEnemy) {
                mBossBarDisplayHealth = mActiveBossEnemy->getHealth() / mActiveBossEnemy->getMaxHealth();
            }

        }
        float targetAlpha = (mActiveBossEnemy != nullptr) ? 1.0f : 0.0f;
        mBossBarVisibleAlpha += (targetAlpha - mBossBarVisibleAlpha) * std::min(1.0f, deltaTime * 4.0f);

        if (mActiveBossEnemy) {
          
            float actualRatio = mActiveBossEnemy->getHealth() / mActiveBossEnemy->getMaxHealth();
            actualRatio = std::clamp(actualRatio, 0.0f, 1.0f);

            if (actualRatio < mBossBarDisplayHealth) {
                                                mBossBarDisplayHealth -= deltaTime * 0.5f;
                if (mBossBarDisplayHealth < actualRatio) mBossBarDisplayHealth = actualRatio;
            }
            else {
                mBossBarDisplayHealth = actualRatio;             }
        }
    }

                static int lastTextureQuality = 0;
    if (mRenderData.rdTextureQuality != lastTextureQuality) {
        lastTextureQuality = mRenderData.rdTextureQuality;
        for (auto& model : mGltfModels) {
            if (model) model->setTextureQuality(mRenderData.rdTextureQuality);
        }
        Logger::log(1, "Graphics settings changed: Texture LOD set to %d\n", mRenderData.rdTextureQuality);
    }
    mGraphicsManager.update(mRenderData, mGltfModels);

    static int lastMSAAIndex = -1;
    if (mRenderData.rdMSAAIndex != lastMSAAIndex) {
        lastMSAAIndex = mRenderData.rdMSAAIndex;
        if (mRenderData.rdMSAAIndex > 0) {
            static const int samplesTable[4] = { 1, 2, 4, 8 };
            mMSFramebuffer.changeSampleCount(samplesTable[mRenderData.rdMSAAIndex]);
        }
        Logger::log(1, "Graphics settings changed: MSAA set to index %d\n", mRenderData.rdMSAAIndex);
    }

                bool useMSAA = (mRenderData.rdMSAAIndex > 0);
    if (useMSAA) {
        mMSFramebuffer.bind();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    else {
        mFramebuffer.bind();
        GLenum drawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, drawBuffers);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    glClearColor(0.52f, 0.84f, 1.0f, 0.81f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                mMatrixGenerateTimer.start();
    mProjectionMatrix = glm::perspective(
        glm::radians(static_cast<float>(mRenderData.rdFieldOfView)),
        static_cast<float>(mRenderData.rdWidth) / static_cast<float>(mRenderData.rdHeight),
        0.01f, 500.0f
    );

                int selectedInstance = mRenderData.rdCurrentSelectedInstance;

    if (mGameState == GameState::MAIN_MENU) {
        mViewMatrix = mMenuController->getCameraViewMatrix();

        {
            std::vector<glm::mat4> matrixData = { mViewMatrix, mProjectionMatrix };
            mUniformBuffer.uploadUboData(matrixData, 0);
        }

        auto njInst = mMenuController->getInstance();
        ModelSettings njSet = njInst->getInstanceSettings();
        mRenderData.rdTriangleCount = static_cast<unsigned int>(njInst->getModel()->getTriangleCount());

        if (njSet.msVertexSkinningMode == skinningMode::dualQuat) {
            std::vector<glm::mat2x4> dq = njInst->getJointDualQuats();
            while (dq.size() < 256) dq.push_back(glm::mat2x4(0.0f));
            mGltfDualQuatSSBuffer.uploadSsboData(dq, 2);
        }
        else {
            std::vector<glm::mat4> mats = njInst->getJointMatrices();
            while (mats.size() < 256) mats.push_back(glm::mat4(1.0f));
            mGltfShaderStorageBuffer.uploadSsboData(mats, 1);
        }

        mRenderData.rdMatrixGenerateTime = mMatrixGenerateTimer.stop();
        mRenderData.rdUploadToUBOTime = 0.0f;
        mRenderData.rdUploadToVBOTime = 0.0f;

                glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mSkyBox.getIrradianceMap());
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mSkyBox.getPrefilterMap());
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, mSkyBox.getBRDFLUTMap());

                bool isDualQuat = (njSet.msVertexSkinningMode == skinningMode::dualQuat);
        bool packed = njInst->getModel()->mHasPackedMap;

        Shader* activeShader = nullptr;
        const char* shaderName = "Unknown";

        if (isDualQuat) {
            if (packed) {
                activeShader = &mGltfGPUDualQuatChannelShader;
                shaderName = "mGltfGPUDualQuatChannelShader (DQ + Packed)";
            }
            else {
                activeShader = &mGltfGPUDualQuatShader;
                shaderName = "mGltfGPUDualQuatShader (DQ + Standard PBR)";
            }
        }
        else {
            if (packed) {
                activeShader = &mGltfGPUShaderToyChannelPack;
                shaderName = "mGltfGPUShaderToyChannelPack (Matrix + Packed)";
            }
            else {
                activeShader = &mGltfGPUShaderToy;
                shaderName = "mGltfGPUShaderToy (Matrix + Standard PBR)";
            }
        }

        static Shader* lastLoggedShader = nullptr;
        if (activeShader != lastLoggedShader) {
            Logger::log(1, "[NJ MENU SHADER] Selected Shader: %s | isDualQuat: %d | hasPackedMap: %d\n",
                shaderName, isDualQuat ? 1 : 0, packed ? 1 : 0);
            lastLoggedShader = activeShader;
        }

        Shader& sh = *activeShader;
        sh.use();

        if (sh.getUniformLocation("u_SunDirection"))   sh.setUniformValue(mRenderData.rdSunDirection);
        if (sh.getUniformLocation("u_SunColor"))       sh.setUniformValue(mRenderData.rdSunColor);
        if (sh.getUniformLocation("u_SunIntensity"))   sh.setUniformValue(mRenderData.rdSunIntensity);
        if (sh.getUniformLocation("u_LightPos"))       sh.setUniformValue(mRenderData.rdLightPos);
        if (sh.getUniformLocation("u_PointColor"))     sh.setUniformValue(mRenderData.rdPointColor);
        if (sh.getUniformLocation("u_PointIntensity")) sh.setUniformValue(mRenderData.rdPointIntensity);

        if (sh.getUniformLocation("u_IrradianceMap"))  sh.setUniformValue(5);
        if (sh.getUniformLocation("u_PrefilterMap"))   sh.setUniformValue(6);
        if (sh.getUniformLocation("u_BRDFLUT"))        sh.setUniformValue(7);

        if (packed) {
            if (sh.getUniformLocation("u_AlbedoMap"))       sh.setUniformValue(0);
            if (sh.getUniformLocation("u_NormalMap"))       sh.setUniformValue(1);
            if (sh.getUniformLocation("u_PackedMap"))       sh.setUniformValue(2);
            if (sh.getUniformLocation("u_CoatRoughMap"))    sh.setUniformValue(3);
            if (sh.getUniformLocation("u_EmitMap"))         sh.setUniformValue(4);

            if (sh.getUniformLocation("u_HasNormalMap"))    sh.setUniformValue(njInst->getModel()->mHasNormalMap ? 1 : 0);
            if (sh.getUniformLocation("u_HasPackedMap"))    sh.setUniformValue(njInst->getModel()->mHasPackedMap ? 1 : 0);
            if (sh.getUniformLocation("u_HasCoatRoughMap")) sh.setUniformValue(njInst->getModel()->mHasCoatRoughMap ? 1 : 0);
            if (sh.getUniformLocation("u_HasEmitMap"))      sh.setUniformValue(njInst->getModel()->mHasEmitMap ? 1 : 0);
                                    float emitIntensity = UserInterface::showEmit ? 3.0f : 0.0f;

            if (sh.getUniformLocation("u_EmissionMultiplier"))
                sh.setUniformValue(emitIntensity);
        }
        else {
            if (sh.getUniformLocation("u_BaseColorMap"))   sh.setUniformValue(0);
            if (sh.getUniformLocation("u_NormalMap"))      sh.setUniformValue(1);
            if (sh.getUniformLocation("u_MetallicMap"))    sh.setUniformValue(2);
            if (sh.getUniformLocation("u_RoughnessMap"))   sh.setUniformValue(3);
        }

        if (sh.getUniformLocation("aModelStride")) sh.setUniformValue(0);
        njInst->getModel()->draw();

        if (UserInterface::showTentacle == true) {
            auto boxInstance = mGltfInstances.at(9);
            boxInstance->getModel()->draw();
        }
                            }
    else if (mGameState == GameState::LOADING) {
        mViewMatrix = glm::mat4(1.0f);
        std::vector<glm::mat4> matrixData = { mViewMatrix, mProjectionMatrix };
        mUniformBuffer.uploadUboData(matrixData, 0);
        mRenderData.rdMatrixGenerateTime = mMatrixGenerateTimer.stop();

                            }
    else {
        glm::vec3 playerPos = mGltfInstances.at(1)->getWorldPosition();
        playerPos.y += 0.8f;
        mViewMatrix = mCamera.getThirdPersonViewMatrix(mRenderData, playerPos);

        mRenderData.rdIKTime = 0.0f;
        for (auto& instance : mGltfInstances) {
            if (instance->getJointMatrixSize() > 0) {
                instance->updateAnimation();
                mIKTimer.start();
                instance->solveIK();
                mRenderData.rdIKTime += static_cast<float>(mIKTimer.stop());
                                if (instance == mGltfInstances.at(1)) {
                    updateBreastJiggle(mLeftBreastJiggle, deltaTime);
                    updateBreastJiggle(mRightBreastJiggle, deltaTime);
                    instance->refreshJointMatrices();                                      }
            }
        }

        glm::vec3 modelWorldPos = mGltfInstances.at(selectedInstance)->getWorldPosition();
        glm::quat modelWorldRot = mGltfInstances.at(selectedInstance)->getWorldRotation();

        mRenderData.rdMatrixGenerateTime = mMatrixGenerateTimer.stop();

        mUploadToUBOTimer.start();
        {
            std::vector<glm::mat4> matrixData = { mViewMatrix, mProjectionMatrix };
            mUniformBuffer.uploadUboData(matrixData, 0);
        }

        mModelJointMatrices.clear();
        mModelJointDualQuats.clear();
        mStaticInstances.clear();
        mGltfMatrixInstances.clear();
        mGltfDQInstances.clear();

        unsigned int numTriangles = 0;
        for (const auto& instance : mGltfInstances) {
            ModelSettings settings = instance->getInstanceSettings();
            if (!settings.msDrawModel) continue;

            if (instance->getJointMatrixSize() == 0) {
                mStaticInstances.emplace_back(instance);
                numTriangles += static_cast<unsigned int>(instance->getModel()->getTriangleCount());
                continue;
            }
            else if (settings.msVertexSkinningMode == skinningMode::dualQuat) {
                std::vector<glm::mat2x4> quats = instance->getJointDualQuats();
                mModelJointDualQuats.insert(mModelJointDualQuats.end(), quats.begin(), quats.end());
                size_t padding = 256 - quats.size();
                for (size_t i = 0; i < padding; ++i)
                    mModelJointDualQuats.push_back(glm::mat2x4(0.0f));
                mGltfDQInstances.emplace_back(instance);
            }
            else {
                std::vector<glm::mat4> mats = instance->getJointMatrices();
                mModelJointMatrices.insert(mModelJointMatrices.end(), mats.begin(), mats.end());
                size_t padding = 256 - mats.size();
                for (size_t i = 0; i < padding; ++i)
                    mModelJointMatrices.push_back(glm::mat4(1.0f));
                mGltfMatrixInstances.emplace_back(instance);
            }
            numTriangles += static_cast<unsigned int>(instance->getModel()->getTriangleCount());
        }
        mRenderData.rdTriangleCount = numTriangles;

        mGltfShaderStorageBuffer.uploadSsboData(mModelJointMatrices, 1);
        mGltfDualQuatSSBuffer.uploadSsboData(mModelJointDualQuats, 2);
        mRenderData.rdUploadToUBOTime = mUploadToUBOTimer.stop();

        mUploadToVBOTimer.start();
        mRenderData.rdUploadToVBOTime = mUploadToVBOTimer.stop();

                glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_CUBE_MAP, mSkyBox.getIrradianceMap());
        glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_CUBE_MAP, mSkyBox.getPrefilterMap());
        glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D, mSkyBox.getBRDFLUTMap());

                mStaticShader.use();
        if (mStaticShader.getUniformLocation("u_SunDirection"))   mStaticShader.setUniformValue(mRenderData.rdSunDirection);
        if (mStaticShader.getUniformLocation("u_SunColor"))       mStaticShader.setUniformValue(mRenderData.rdSunColor);
        if (mStaticShader.getUniformLocation("u_SunIntensity"))   mStaticShader.setUniformValue(mRenderData.rdSunIntensity);
        if (mStaticShader.getUniformLocation("u_LightPos"))       mStaticShader.setUniformValue(mRenderData.rdLightPos);
        if (mStaticShader.getUniformLocation("u_PointColor"))     mStaticShader.setUniformValue(mRenderData.rdPointColor);
        if (mStaticShader.getUniformLocation("u_PointIntensity")) mStaticShader.setUniformValue(mRenderData.rdPointIntensity);
        if (mStaticShader.getUniformLocation("u_BaseColorMap"))   mStaticShader.setUniformValue(0);
        if (mStaticShader.getUniformLocation("u_NormalMap"))      mStaticShader.setUniformValue(1);
        if (mStaticShader.getUniformLocation("u_MetallicMap"))    mStaticShader.setUniformValue(2);
        if (mStaticShader.getUniformLocation("u_RoughnessMap"))   mStaticShader.setUniformValue(3);
        if (mStaticShader.getUniformLocation("u_EmitMap"))        mStaticShader.setUniformValue(4);
        if (mStaticShader.getUniformLocation("u_IrradianceMap"))  mStaticShader.setUniformValue(5);
        if (mStaticShader.getUniformLocation("u_PrefilterMap"))   mStaticShader.setUniformValue(6);
        if (mStaticShader.getUniformLocation("u_BRDFLUT"))        mStaticShader.setUniformValue(7);

        for (const auto& instance : mStaticInstances) {
            instance->getRootNode()->calculateNodeMatrix();
            glm::mat4 modelMatrix = instance->getRootNode()->getNodeMatrix();
            if (mStaticShader.getUniformLocation("u_Model"))        mStaticShader.setUniformValue(modelMatrix);
            if (mStaticShader.getUniformLocation("u_HasNormalMap")) mStaticShader.setUniformValue(instance->getModel()->mHasNormalMap ? 1 : 0);
            if (mStaticShader.getUniformLocation("u_HasMetalMap"))  mStaticShader.setUniformValue(instance->getModel()->mHasMetalMap ? 1 : 0);
            if (mStaticShader.getUniformLocation("u_HasRoughMap"))  mStaticShader.setUniformValue(instance->getModel()->mHasRoughMap ? 1 : 0);
            if (mStaticShader.getUniformLocation("u_HasEmitMap"))   mStaticShader.setUniformValue(instance->getModel()->mHasEmitMap ? 1 : 0);
            instance->getModel()->draw();
        }

                unsigned int matrixPos = 0;
        int instanceCounter = 0;
        for (const auto& instance : mGltfMatrixInstances) {
            if (instanceCounter == 0) {
                if (mRenderData.rdEntyAnim == 0) {
                    mGltfGPUShaderToy.use();
                    GltfGPUShaderToy(matrixPos);
                }
                else {
                    mGltfGPUShader.use();
                    if (mGltfGPUShader.getUniformLocation("aModelStride")) mGltfGPUShader.setUniformValue((int)matrixPos);
                }
            }
            else if (instanceCounter == 14) {
                GltfGPUShaderToyChannelPack(matrixPos, instance);
            }
            else {
                mGltfGPUShader.use();
                if (mGltfGPUShader.getUniformLocation("aModelStride")) mGltfGPUShader.setUniformValue((int)matrixPos);
            }
            instance->getModel()->draw();
            matrixPos += static_cast<unsigned int>(256);
            instanceCounter++;
        }

                unsigned int dqPos = 0;
        for (const auto& instance : mGltfDQInstances) {
            if (instance->getModel()->mHasPackedMap) {
                mGltfGPUDualQuatChannelShader.use();
                if (mGltfGPUDualQuatChannelShader.getUniformLocation("u_SunDirection"))    mGltfGPUDualQuatChannelShader.setUniformValue(mRenderData.rdSunDirection);
                if (mGltfGPUDualQuatChannelShader.getUniformLocation("u_SunColor"))        mGltfGPUDualQuatChannelShader.setUniformValue(mRenderData.rdSunColor);
                if (mGltfGPUDualQuatChannelShader.getUniformLocation("u_SunIntensity"))    mGltfGPUDualQuatChannelShader.setUniformValue(mRenderData.rdSunIntensity);
                if (mGltfGPUDualQuatChannelShader.getUniformLocation("u_LightPos"))        mGltfGPUDualQuatChannelShader.setUniformValue(mRenderData.rdLightPos);
                if (mGltfGPUDualQuatChannelShader.getUniformLocation("u_PointColor"))      mGltfGPUDualQuatChannelShader.setUniformValue(mRenderData.rdPointColor);
                if (mGltfGPUDualQuatChannelShader.getUniformLocation("u_PointIntensity"))  mGltfGPUDualQuatChannelShader.setUniformValue(mRenderData.rdPointIntensity);
                if (mGltfGPUDualQuatChannelShader.getUniformLocation("u_AlbedoMap"))       mGltfGPUDualQuatChannelShader.setUniformValue(0);
                if (mGltfGPUDualQuatChannelShader.getUniformLocation("u_NormalMap"))       mGltfGPUDualQuatChannelShader.setUniformValue(1);
                if (mGltfGPUDualQuatChannelShader.getUniformLocation("u_PackedMap"))       mGltfGPUDualQuatChannelShader.setUniformValue(2);
                if (mGltfGPUDualQuatChannelShader.getUniformLocation("u_CoatRoughMap"))    mGltfGPUDualQuatChannelShader.setUniformValue(3);
                if (mGltfGPUDualQuatChannelShader.getUniformLocation("u_EmitMap"))         mGltfGPUDualQuatChannelShader.setUniformValue(4);
                if (mGltfGPUDualQuatChannelShader.getUniformLocation("u_IrradianceMap"))   mGltfGPUDualQuatChannelShader.setUniformValue(5);
                if (mGltfGPUDualQuatChannelShader.getUniformLocation("u_PrefilterMap"))    mGltfGPUDualQuatChannelShader.setUniformValue(6);
                if (mGltfGPUDualQuatChannelShader.getUniformLocation("u_BRDFLUT"))         mGltfGPUDualQuatChannelShader.setUniformValue(7);
                if (mGltfGPUDualQuatChannelShader.getUniformLocation("u_HasNormalMap"))    mGltfGPUDualQuatChannelShader.setUniformValue(instance->getModel()->mHasNormalMap ? 1 : 0);
                if (mGltfGPUDualQuatChannelShader.getUniformLocation("u_HasPackedMap"))    mGltfGPUDualQuatChannelShader.setUniformValue(instance->getModel()->mHasPackedMap ? 1 : 0);
                if (mGltfGPUDualQuatChannelShader.getUniformLocation("u_HasCoatRoughMap")) mGltfGPUDualQuatChannelShader.setUniformValue(instance->getModel()->mHasCoatRoughMap ? 1 : 0);
                if (mGltfGPUDualQuatChannelShader.getUniformLocation("u_HasEmitMap"))      mGltfGPUDualQuatChannelShader.setUniformValue(instance->getModel()->mHasEmitMap ? 1 : 0);
                if (mGltfGPUDualQuatChannelShader.getUniformLocation("aModelStride"))      mGltfGPUDualQuatChannelShader.setUniformValue((int)dqPos);
            }
            else {
                mGltfGPUDualQuatShader.use();
                if (mGltfGPUDualQuatShader.getUniformLocation("u_SunDirection"))   mGltfGPUDualQuatShader.setUniformValue(mRenderData.rdSunDirection);
                if (mGltfGPUDualQuatShader.getUniformLocation("u_SunColor"))       mGltfGPUDualQuatShader.setUniformValue(mRenderData.rdSunColor);
                if (mGltfGPUDualQuatShader.getUniformLocation("u_SunIntensity"))   mGltfGPUDualQuatShader.setUniformValue(mRenderData.rdSunIntensity);
                if (mGltfGPUDualQuatShader.getUniformLocation("u_LightPos"))       mGltfGPUDualQuatShader.setUniformValue(mRenderData.rdLightPos);
                if (mGltfGPUDualQuatShader.getUniformLocation("u_PointColor"))     mGltfGPUDualQuatShader.setUniformValue(mRenderData.rdPointColor);
                if (mGltfGPUDualQuatShader.getUniformLocation("u_PointIntensity")) mGltfGPUDualQuatShader.setUniformValue(mRenderData.rdPointIntensity);
                if (mGltfGPUDualQuatShader.getUniformLocation("u_BaseColorMap"))   mGltfGPUDualQuatShader.setUniformValue(0);
                if (mGltfGPUDualQuatShader.getUniformLocation("u_NormalMap"))      mGltfGPUDualQuatShader.setUniformValue(1);
                if (mGltfGPUDualQuatShader.getUniformLocation("u_MetallicMap"))    mGltfGPUDualQuatShader.setUniformValue(2);
                if (mGltfGPUDualQuatShader.getUniformLocation("u_RoughnessMap"))   mGltfGPUDualQuatShader.setUniformValue(3);
                if (mGltfGPUDualQuatShader.getUniformLocation("u_IrradianceMap"))  mGltfGPUDualQuatShader.setUniformValue(5);
                if (mGltfGPUDualQuatShader.getUniformLocation("u_PrefilterMap"))   mGltfGPUDualQuatShader.setUniformValue(6);
                if (mGltfGPUDualQuatShader.getUniformLocation("u_BRDFLUT"))        mGltfGPUDualQuatShader.setUniformValue(7);
                if (mGltfGPUDualQuatShader.getUniformLocation("aModelStride"))     mGltfGPUDualQuatShader.setUniformValue((int)dqPos);
            }
            instance->getModel()->draw();
            dqPos += instance->getJointDualQuatsSize();
        }
    } 
                
    if (mGameState != GameState::LOADING) {
        mAccumulatedTime += mRenderData.rdFrameTime;
        float cameraYaw = std::atan2(mViewMatrix[2][0], mViewMatrix[2][2]);
        glm::mat4 skyboxView = glm::mat4(glm::mat3(mViewMatrix));
        mSkyBox.draw(skyboxView, mProjectionMatrix, cameraYaw, mAccumulatedTime);
    }

                if (mGameState == GameState::GAMEPLAY && mBossBarVisibleAlpha > 0.01f && mActiveBossEnemy) {
        if (mCoordArrowsLineIndexCount > 0) {
            mLineShader.use();
            mVertexBuffer.bindAndDraw(GL_LINES, mSkeletonLineIndexCount, mCoordArrowsLineIndexCount);
        }
        if (mSkeletonLineIndexCount > 0) {
            glDisable(GL_DEPTH_TEST);
            mLineShader.use();
            mVertexBuffer.bindAndDraw(GL_LINES, 0, mSkeletonLineIndexCount);
            glEnable(GL_DEPTH_TEST);
        }
    }

                if (useMSAA) {
        mMSFramebuffer.unbind();
        mMSFramebuffer.resolveTo(mFramebuffer);
    }
    else {
        mFramebuffer.unbind();
    }
                glDisable(GL_DEPTH_TEST);

    mBloomBlurShader.use();
    GLuint blurProg = mBloomBlurShader.getHandle();
    glBindVertexArray(mBloomVAO);

    bool horizontal = true, firstIteration = true;
    for (int i = 0; i < mBloomPasses; ++i) {
        int wIdx = horizontal ? 0 : 1, rIdx = horizontal ? 1 : 0;
        mBlurFBO[wIdx].bind();
        glClear(GL_COLOR_BUFFER_BIT);
        glUniform1i(glGetUniformLocation(blurProg, "u_Horizontal"), horizontal ? 1 : 0);
        glUniform1i(glGetUniformLocation(blurProg, "u_Image"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,
            firstIteration ? mFramebuffer.getEmissionTex() : mBlurFBO[rIdx].getColorTex());
        glDrawArrays(GL_TRIANGLES, 0, 3);
        mBlurFBO[wIdx].unbind();
        horizontal = !horizontal; firstIteration = false;
    }
    int lastBlur = horizontal ? 1 : 0;

        if (mShowDeathScreen) {
        mDeathCaptureFBO.bind();
        glViewport(0, 0, mRenderData.rdWidth, mRenderData.rdHeight);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, mRenderData.rdWidth, mRenderData.rdHeight);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    mBloomCompositeShader.use();
    GLuint compProg = mBloomCompositeShader.getHandle();
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, mFramebuffer.getColorTex());
    glUniform1i(glGetUniformLocation(compProg, "u_Scene"), 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, mBlurFBO[lastBlur].getColorTex());
    glUniform1i(glGetUniformLocation(compProg, "u_BloomBlur"), 1);
    glUniform1f(glGetUniformLocation(compProg, "u_BloomStrength"), mBloomStrength);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    if (mShowDeathScreen) {
        mDeathCaptureFBO.unbind();

                        int deathBlurPasses = 5;
        bool dHorizontal = true, dFirst = true;
        for (int i = 0; i < deathBlurPasses; ++i) {
            int wIdx = dHorizontal ? 0 : 1, rIdx = dHorizontal ? 1 : 0;
            mBlurFBO[wIdx].bind();
            glClear(GL_COLOR_BUFFER_BIT);
            glUniform1i(glGetUniformLocation(blurProg, "u_Horizontal"), dHorizontal ? 1 : 0);
            glUniform1i(glGetUniformLocation(blurProg, "u_Image"), 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D,
                dFirst ? mDeathCaptureFBO.getColorTex() : mBlurFBO[rIdx].getColorTex());
            glDrawArrays(GL_TRIANGLES, 0, 3);
            mBlurFBO[wIdx].unbind();
            dHorizontal = !dHorizontal; dFirst = false;
        }
        int lastDeathBlur = dHorizontal ? 1 : 0;

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, mRenderData.rdWidth, mRenderData.rdHeight);
        glClear(GL_COLOR_BUFFER_BIT);

        mBloomCompositeShader.use();
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, mBlurFBO[lastDeathBlur].getColorTex());
        glUniform1i(glGetUniformLocation(compProg, "u_Scene"), 0);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, mBlurFBO[lastDeathBlur].getColorTex());
        glUniform1i(glGetUniformLocation(compProg, "u_BloomBlur"), 1);
        glUniform1f(glGetUniformLocation(compProg, "u_BloomStrength"), 0.0f);         glDrawArrays(GL_TRIANGLES, 0, 3);


        
    }

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);

                mUIGenerateTimer.start();

    if (mGameState == GameState::MAIN_MENU) {
        ModelSettings dummy{};
        mUserInterface.createFrame(mRenderData, dummy, false, false);

        MenuAction uiAction = mMenuController->drawUI(mRenderData);
        if (UserInterface::showSettingsWindow) {
            UserInterface::drawSettingsOverlay(mRenderData, UserInterface::showSettingsWindow);
        }
        if (uiAction == MenuAction::QUIT) {
            glfwSetWindowShouldClose(mRenderData.rdWindow, GLFW_TRUE);
        }
    }
    else if (mGameState == GameState::LOADING) {
        ModelSettings dummy{};
        mUserInterface.createFrame(mRenderData, dummy, false, false);
    }
    else {
        ModelSettings settings = mGltfInstances.at(selectedInstance)->getInstanceSettings();
        mUserInterface.createFrame(mRenderData, settings, mShowMenu, true);
      
        if (mBossBarVisibleAlpha > 0.001f && mActiveBossEnemy) {
            ImDrawList* dl = ImGui::GetForegroundDrawList();
            float W = static_cast<float>(mRenderData.rdWidth);
            float H = static_cast<float>(mRenderData.rdHeight);
            float time = static_cast<float>(ImGui::GetTime());
            float deltaTime = ImGui::GetIO().DeltaTime;

                                                struct EmberParticle {
                ImVec2 pos;
                ImVec2 vel;
                float  life;
                float  maxLife;
                float  size;
                float  wobbleSpeed;
                ImU32  color;
            };

            struct DamagePopup {
                ImVec2 pos;
                float  damage;
                float  life;
                float  maxLife;
            };

            static float sSmoothedHp = 1.0f;
            static float sChipHp = 1.0f;
            static float sLastHp = 1.0f;
            static float sShakeAmount = 0.0f;
            static std::vector<EmberParticle> sEmbers;
            static std::vector<DamagePopup>   sDamagePopups;

            float realHp = std::clamp(mActiveBossEnemy->getHealth() / mActiveBossEnemy->getMaxHealth(), 0.0f, 1.0f);
            bool isLordOfCinderPhase = (realHp <= 0.50f); 
                        sSmoothedHp = ImLerp(sSmoothedHp, realHp, deltaTime * 12.0f);
            sChipHp = ImLerp(sChipHp, realHp, deltaTime * 1.5f);

                                                float deltaHp = sLastHp - realHp;
            if (deltaHp > 0.0001f) {
                sShakeAmount = std::min(sShakeAmount + deltaHp * 25.0f, 8.0f); 
                float barWidthTemp = W * 0.42f;
                float barMinXTemp = (W * 0.5f) - (barWidthTemp * 0.5f);
                float impactX = barMinXTemp + barWidthTemp * realHp;
                float barYTemp = H - 70.0f;

                                DamagePopup popup;
                popup.pos = ImVec2(impactX + ((rand() % 20) - 10), barYTemp - 26.0f);
                popup.damage = deltaHp * mActiveBossEnemy->getMaxHealth();
                popup.maxLife = 0.9f;
                popup.life = popup.maxLife;
                sDamagePopups.push_back(popup);

                                int particleCount = static_cast<int>(deltaHp * 250.0f) + 8;
                particleCount = std::min(particleCount, 45);

                for (int i = 0; i < particleCount; ++i) {
                    EmberParticle p;
                    p.pos = ImVec2(impactX + ((rand() % 12) - 6), barYTemp + ((rand() % 12) - 6));
                    float angle = -3.14159f * 0.5f + ((rand() % 100) / 100.0f - 0.5f) * 1.5f;
                    float speed = 20.0f + (rand() % 90);
                    p.vel = ImVec2(cosf(angle) * speed, sinf(angle) * speed);
                    p.maxLife = 0.6f + ((rand() % 100) / 100.0f) * 0.8f;
                    p.life = p.maxLife;
                    p.size = 1.0f + ((rand() % 100) / 100.0f) * 2.2f;
                    p.wobbleSpeed = 2.0f + ((rand() % 100) / 100.0f) * 4.0f;

                    int r = 230 + (rand() % 25);
                    int g = 80 + (rand() % 100);
                    int b = 15;
                    p.color = IM_COL32(r, g, b, 255);
                    sEmbers.push_back(p);
                }
            }
            sLastHp = realHp;

                        sShakeAmount = ImLerp(sShakeAmount, 0.0f, deltaTime * 10.0f);
            ImVec2 shakeOffset(
                ((rand() % 100) / 100.0f - 0.5f) * sShakeAmount,
                ((rand() % 100) / 100.0f - 0.5f) * sShakeAmount
            );

                        if (isLordOfCinderPhase && sEmbers.size() < 60 && (rand() % 4 == 0)) {
                EmberParticle p;
                float barWidthTemp = W * 0.42f;
                float barMinXTemp = (W * 0.5f) - (barWidthTemp * 0.5f);
                p.pos = ImVec2(barMinXTemp + ((rand() % static_cast<int>(barWidthTemp))), (H - 70.0f) + (rand() % 10));
                p.vel = ImVec2(((rand() % 100) / 100.0f - 0.5f) * 10.0f, -15.0f - (rand() % 25));
                p.maxLife = 1.0f + ((rand() % 100) / 100.0f) * 1.0f;
                p.life = p.maxLife;
                p.size = 1.0f + ((rand() % 100) / 100.0f) * 1.5f;
                p.wobbleSpeed = 1.5f + ((rand() % 100) / 100.0f) * 3.0f;
                p.color = IM_COL32(240, 110, 20, 255);
                sEmbers.push_back(p);
            }

                        for (size_t i = 0; i < sEmbers.size();) {
                sEmbers[i].life -= deltaTime;
                if (sEmbers[i].life <= 0.0f) {
                    sEmbers[i] = sEmbers.back();
                    sEmbers.pop_back();
                }
                else {
                    sEmbers[i].pos.x += (sEmbers[i].vel.x + sinf(time * sEmbers[i].wobbleSpeed) * 12.0f) * deltaTime;
                    sEmbers[i].pos.y += sEmbers[i].vel.y * deltaTime;
                    ++i;
                }
            }

                        for (size_t i = 0; i < sDamagePopups.size();) {
                sDamagePopups[i].life -= deltaTime;
                if (sDamagePopups[i].life <= 0.0f) {
                    sDamagePopups[i] = sDamagePopups.back();
                    sDamagePopups.pop_back();
                }
                else {
                    sDamagePopups[i].pos.y -= 15.0f * deltaTime;
                    ++i;
                }
            }

                                                float alpha = mBossBarVisibleAlpha;
            int a = static_cast<int>(alpha * 255.0f);

            float barW = W * 0.42f;             float barH = 14.0f;                 float cx = (W * 0.5f) + shakeOffset.x;
            float barY = (H - 70.0f) + shakeOffset.y;

            ImVec2 barMin(cx - barW * 0.5f, barY);
            ImVec2 barMax(cx + barW * 0.5f, barY + barH);

                                                            dl->AddRectFilled(
                ImVec2(barMin.x - 4.0f, barMin.y - 4.0f),
                ImVec2(barMax.x + 4.0f, barMax.y + 4.0f),
                IM_COL32(5, 5, 5, static_cast<int>(a * 0.9f))
            );

                        dl->AddRectFilled(barMin, barMax, IM_COL32(18, 15, 15, a));

                                                float chipWidth = barW * sChipHp;
            float realWidth = barW * sSmoothedHp;

            if (chipWidth > realWidth + 0.5f) {
                ImVec2 chipMin(barMin.x + realWidth, barMin.y);
                ImVec2 chipMax(barMin.x + chipWidth, barMax.y);

                dl->AddRectFilledMultiColor(
                    chipMin, chipMax,
                    IM_COL32(210, 140, 40, a),
                    IM_COL32(160, 90, 20, a),
                    IM_COL32(160, 90, 20, a),
                    IM_COL32(210, 140, 40, a)
                );
            }

                                                if (realWidth > 0.5f) {
                ImVec2 hpMax(barMin.x + realWidth, barMax.y);

                ImU32 colTop = isLordOfCinderPhase ? IM_COL32(235, 45, 20, a) : IM_COL32(185, 25, 22, a);
                ImU32 colBot = isLordOfCinderPhase ? IM_COL32(140, 12, 8, a) : IM_COL32(100, 8, 8, a);

                dl->AddRectFilledMultiColor(barMin, hpMax, colTop, colTop, colBot, colBot);

                                dl->AddLine(
                    ImVec2(barMin.x, barMin.y + 1.5f),
                    ImVec2(hpMax.x, barMin.y + 1.5f),
                    IM_COL32(255, 170, 130, static_cast<int>(a * 0.45f)), 1.5f
                );
            }

                        ImU32 frameBorderCol = IM_COL32(140, 115, 75, static_cast<int>(a * 0.85f));
            ImU32 innerBorderCol = IM_COL32(40, 32, 22, static_cast<int>(a * 0.6f));

            dl->AddRect(barMin, barMax, frameBorderCol, 0.0f, 0, 1.5f);
            dl->AddRect(ImVec2(barMin.x - 2.0f, barMin.y - 2.0f), ImVec2(barMax.x + 2.0f, barMax.y + 2.0f), innerBorderCol, 0.0f, 0, 1.0f);

                                                for (const auto& p : sEmbers) {
                float pAlpha = (p.life / p.maxLife) * alpha;
                ImU32 c = (p.color & 0x00FFFFFF) | (static_cast<int>(pAlpha * 255.0f) << 24);
                dl->AddCircleFilled(p.pos, p.size, c, 6);
            }

                                                std::string bossName = mActiveBossEnemy->getBossName();
                        ImVec2 namePos(barMin.x + 2.0f, barMin.y - 24.0f);

                        dl->AddText(ImVec2(namePos.x + 1.5f, namePos.y + 1.5f), IM_COL32(0, 0, 0, static_cast<int>(a * 0.95f)), bossName.c_str());

                        ImU32 nameColor = IM_COL32(230, 220, 195, a);
            dl->AddText(namePos, nameColor, bossName.c_str());

                                                for (const auto& popup : sDamagePopups) {
                float pAlpha = (popup.life / popup.maxLife) * alpha;
                char dmgBuf[16];
                snprintf(dmgBuf, sizeof(dmgBuf), "%.0f", popup.damage);

                ImU32 dmgCol = IM_COL32(245, 225, 185, static_cast<int>(pAlpha * 255.0f));
                dl->AddText(ImVec2(popup.pos.x + 1.0f, popup.pos.y + 1.0f), IM_COL32(0, 0, 0, static_cast<int>(pAlpha * 255.0f)), dmgBuf);
                dl->AddText(popup.pos, dmgCol, dmgBuf);
            }

                                                float midY = barY + barH * 0.5f;

                        ImVec2 leftDiamond[4] = {
                ImVec2(barMin.x - 10.0f, midY),
                ImVec2(barMin.x - 5.0f, midY - 4.0f),
                ImVec2(barMin.x, midY),
                ImVec2(barMin.x - 5.0f, midY + 4.0f)
            };
            dl->AddConvexPolyFilled(leftDiamond, 4, frameBorderCol);

                        ImVec2 rightDiamond[4] = {
                ImVec2(barMax.x, midY),
                ImVec2(barMax.x + 5.0f, midY - 4.0f),
                ImVec2(barMax.x + 10.0f, midY),
                ImVec2(barMax.x + 5.0f, midY + 4.0f)
            };
            dl->AddConvexPolyFilled(rightDiamond, 4, frameBorderCol);
        }
        mGltfInstances.at(selectedInstance)->setInstanceSettings(settings);
        mGltfInstances.at(selectedInstance)->checkForUpdates();
    }

        if (mDeathFadeAlpha > 0.001f) {
        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        ImVec2 displaySize = ImGui::GetIO().DisplaySize;
        float W = displaySize.x;
        float H = displaySize.y;
        float alphaProgress = mDeathFadeAlpha / DEATH_FADE_MAX;

                ImU32 colBG = IM_COL32(5, 5, 10, static_cast<int>(245.0f * alphaProgress));
        dl->AddRectFilled(ImVec2(0, 0), ImVec2(W, H), colBG);

        if (alphaProgress > 0.2f) {

            float uiAlpha = (alphaProgress - 0.2f) / 0.8f;
            
            static float fadeInTimer = 0.0f;
            float fadeInDuration = 1.0f; 
                        fadeInTimer += ImGui::GetIO().DeltaTime;
            if (fadeInTimer > fadeInDuration) fadeInTimer = fadeInDuration;

                        float progress = fadeInTimer / fadeInDuration;
            float easeAlpha = progress * (2.0f - progress); 
                        float finalAlpha = uiAlpha * easeAlpha;

                        float time = static_cast<float>(ImGui::GetTime());
            float glowPulse = 0.85f + 0.15f * sinf(time * 3.0f);
            bool hasCustomFont = (ImGui::GetIO().Fonts->Fonts.Size > 1);
            if (hasCustomFont) ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

            const char* title = "YOU DIED";
            float fontSize = 120.0f;

                        float dynamicScaleY = 1.6f + (1.0f - easeAlpha) * 0.2f;

            ImVec2 tSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, title);
            tSize.y *= dynamicScaleY;

            ImVec2 tPos = ImVec2((W - tSize.x) * 0.5f, H * 0.40f - tSize.y);

            int vtxStart = dl->VtxBuffer.Size;

                        for (int i = 1; i <= 6; ++i) {
                float offset = static_cast<float>(i) * 1.5f;
                int alpha = static_cast<int>((70 - (i * 10)) * finalAlpha * glowPulse);
                if (alpha <= 0) break;

                ImU32 glowCol = IM_COL32(0, 150, 255, alpha);
                dl->AddText(ImGui::GetFont(), fontSize, ImVec2(tPos.x - offset, tPos.y), glowCol, title);
                dl->AddText(ImGui::GetFont(), fontSize, ImVec2(tPos.x + offset, tPos.y), glowCol, title);
                dl->AddText(ImGui::GetFont(), fontSize, ImVec2(tPos.x, tPos.y - offset), glowCol, title);
                dl->AddText(ImGui::GetFont(), fontSize, ImVec2(tPos.x, tPos.y + offset), glowCol, title);
            }

                        dl->AddText(ImGui::GetFont(), fontSize, tPos, IM_COL32(240, 250, 255, static_cast<int>(255 * finalAlpha)), title);

            int vtxEnd = dl->VtxBuffer.Size;

            for (int i = vtxStart; i < vtxEnd; ++i) {
                ImDrawVert& vert = dl->VtxBuffer[i];
                vert.pos.y = tPos.y + (vert.pos.y - tPos.y) * dynamicScaleY;
            }

            if (hasCustomFont) ImGui::PopFont();
                        const char* subTitle = "You are dead";
            float subFontSize = ImGui::GetFontSize() * 0.9f;
            ImVec2 subSize = ImGui::GetFont()->CalcTextSizeA(subFontSize, FLT_MAX, 0.0f, subTitle);
            ImVec2 subPos = ImVec2((W - subSize.x) * 0.5f, tPos.y + tSize.y + 10.0f);
            dl->AddText(ImGui::GetFont(), subFontSize, subPos, IM_COL32(150, 200, 255, static_cast<int>(180 * uiAlpha)), subTitle);


                        float iconSize = 36.0f;
            ImVec2 iconPos((W - iconSize) * 0.5f, H * 0.72f);
            ImVec2 iconCenter(iconPos.x + iconSize * 0.5f, iconPos.y + iconSize * 0.5f);

            ImVec2 mousePos = ImGui::GetMousePos();
            bool isHovered = (mousePos.x >= iconPos.x - 25.0f && mousePos.x <= iconPos.x + iconSize + 25.0f &&
                mousePos.y >= iconPos.y - 20.0f && mousePos.y <= iconPos.y + iconSize + 30.0f);

            float alphaMult = isHovered ? 1.4f : 1.0f;
            

                                                            float glowSize1 = 90.0f;
            ImU32 colCenter1 = IM_COL32(0, 80, 255, static_cast<int>(130 * uiAlpha * alphaMult));
            ImU32 colEdge1 = IM_COL32(0, 40, 200, 0);

            dl->AddRectFilledMultiColor(ImVec2(iconCenter.x - glowSize1, iconCenter.y - glowSize1), iconCenter, colEdge1, colEdge1, colCenter1, colEdge1);
            dl->AddRectFilledMultiColor(ImVec2(iconCenter.x, iconCenter.y - glowSize1), ImVec2(iconCenter.x + glowSize1, iconCenter.y), colEdge1, colEdge1, colEdge1, colCenter1);
            dl->AddRectFilledMultiColor(ImVec2(iconCenter.x - glowSize1, iconCenter.y), ImVec2(iconCenter.x, iconCenter.y + glowSize1), colEdge1, colCenter1, colEdge1, colEdge1);
            dl->AddRectFilledMultiColor(iconCenter, ImVec2(iconCenter.x + glowSize1, iconCenter.y + glowSize1), colCenter1, colEdge1, colEdge1, colEdge1);

                        float glowSize2 = 45.0f;
            ImU32 colCenter2 = IM_COL32(0, 230, 255, static_cast<int>(200 * uiAlpha * alphaMult));
            ImU32 colEdge2 = IM_COL32(0, 180, 255, 0);

            dl->AddRectFilledMultiColor(ImVec2(iconCenter.x - glowSize2, iconCenter.y - glowSize2), iconCenter, colEdge2, colEdge2, colCenter2, colEdge2);
            dl->AddRectFilledMultiColor(ImVec2(iconCenter.x, iconCenter.y - glowSize2), ImVec2(iconCenter.x + glowSize2, iconCenter.y), colEdge2, colEdge2, colEdge2, colCenter2);
            dl->AddRectFilledMultiColor(ImVec2(iconCenter.x - glowSize2, iconCenter.y), ImVec2(iconCenter.x, iconCenter.y + glowSize2), colEdge2, colCenter2, colEdge2, colEdge2);
            dl->AddRectFilledMultiColor(iconCenter, ImVec2(iconCenter.x + glowSize2, iconCenter.y + glowSize2), colCenter2, colEdge2, colEdge2, colEdge2);


                                                float ringRadius = 42.0f;
            int numTicks = 24;
            float angleOffset = time * 1.2f;

            for (int i = 0; i < numTicks; ++i) {
                float angle = angleOffset + (static_cast<float>(i) / numTicks) * 2.0f * 3.14159265f;
                float innerR = ringRadius;
                float outerR = ringRadius + ((i % 4 == 0) ? 9.0f : 5.0f);

                ImVec2 pInner(iconCenter.x + cosf(angle) * innerR, iconCenter.y + sinf(angle) * innerR);
                ImVec2 pOuter(iconCenter.x + cosf(angle) * outerR, iconCenter.y + sinf(angle) * outerR);

                float wave = (sinf(angle * 3.0f + time * 4.0f) + 1.0f) * 0.5f;
                int aVal = static_cast<int>((120 + wave * 135) * uiAlpha);

                                dl->AddLine(pInner, pOuter, IM_COL32(0, 140, 255, aVal), 3.5f);
                                dl->AddLine(pInner, pOuter, IM_COL32(180, 255, 255, aVal), 1.2f);
            }


                                                int arcSegments = 3;
            float arcLength = (2.0f * 3.14159265f) / arcSegments;
            float arcOffset = -time * 0.9f;

            for (int i = 0; i < arcSegments; ++i) {
                float startA = arcOffset + i * arcLength;
                float endA = startA + arcLength * 0.65f;

                                dl->PathClear();
                dl->PathArcTo(iconCenter, ringRadius + 9.0f, startA, endA, 20);
                dl->PathStroke(IM_COL32(0, 100, 255, static_cast<int>(180 * uiAlpha)), 0, 7.0f);

                                dl->PathClear();
                dl->PathArcTo(iconCenter, ringRadius + 9.0f, startA, endA, 20);
                dl->PathStroke(IM_COL32(0, 240, 255, static_cast<int>(255 * uiAlpha)), 0, 3.0f);

                                dl->PathClear();
                dl->PathArcTo(iconCenter, ringRadius + 9.0f, startA, endA, 20);
                dl->PathStroke(IM_COL32(230, 255, 255, static_cast<int>(255 * uiAlpha)), 0, 1.0f);
            }


                                                float flareWidth = 200.0f;

                        dl->AddRectFilledMultiColor(
                ImVec2(iconCenter.x - flareWidth, iconCenter.y - 3.0f), ImVec2(iconCenter.x, iconCenter.y + 3.0f),
                IM_COL32(0, 100, 255, 0), IM_COL32(0, 220, 255, static_cast<int>(220 * uiAlpha)),
                IM_COL32(0, 220, 255, static_cast<int>(220 * uiAlpha)), IM_COL32(0, 100, 255, 0));

            dl->AddRectFilledMultiColor(
                ImVec2(iconCenter.x, iconCenter.y - 3.0f), ImVec2(iconCenter.x + flareWidth, iconCenter.y + 3.0f),
                IM_COL32(0, 220, 255, static_cast<int>(220 * uiAlpha)), IM_COL32(0, 100, 255, 0),
                IM_COL32(0, 100, 255, 0), IM_COL32(0, 220, 255, static_cast<int>(220 * uiAlpha)));

                        dl->AddLine(
                ImVec2(iconCenter.x - flareWidth * 0.6f, iconCenter.y),
                ImVec2(iconCenter.x + flareWidth * 0.6f, iconCenter.y),
                IM_COL32(255, 255, 255, static_cast<int>(255 * uiAlpha)), 1.5f);


                                                float outerRingRadius = 18.0f;
            float innerSquareSize = 9.0f;

                        dl->AddCircle(iconCenter, outerRingRadius + 4.0f, IM_COL32(0, 220, 255, static_cast<int>(255 * uiAlpha)), 32, 3.5f);
            dl->AddCircle(iconCenter, outerRingRadius + 1.5f, IM_COL32(220, 255, 255, static_cast<int>(255 * uiAlpha)), 32, 1.5f);

                        dl->AddCircleFilled(iconCenter, outerRingRadius, IM_COL32(255, 255, 255, static_cast<int>(255 * uiAlpha)), 32);

                        ImVec2 sqMin(iconCenter.x - innerSquareSize * 0.5f, iconCenter.y - innerSquareSize * 0.5f);
            ImVec2 sqMax(iconCenter.x + innerSquareSize * 0.5f, iconCenter.y + innerSquareSize * 0.5f);
            dl->AddRect(sqMin, sqMax, IM_COL32(10, 15, 25, static_cast<int>(255 * uiAlpha)), 0.0f, 0, 2.5f);


                                                const char* btnText = "Revive";
            float btnTextSize = ImGui::GetFontSize() * 1.0f;
            ImVec2 bSize = ImGui::GetFont()->CalcTextSizeA(btnTextSize, FLT_MAX, 0.0f, btnText);

                        dl->AddText(ImGui::GetFont(), btnTextSize,
                ImVec2(iconCenter.x - bSize.x * 0.5f + 1.0f, iconCenter.y + ringRadius + 18.0f + 1.0f),
                IM_COL32(0, 0, 0, static_cast<int>(200 * uiAlpha)), btnText);

                        dl->AddText(ImGui::GetFont(), btnTextSize,
                ImVec2(iconCenter.x - bSize.x * 0.5f, iconCenter.y + ringRadius + 18.0f),
                IM_COL32(180, 240, 255, static_cast<int>(255 * uiAlpha)), btnText);

            if (isHovered && ImGui::IsMouseClicked(0)) {
                mRespawnRequested = true;
            }
        }
    }

        if (mFadeAlpha > 0.001f) {
        ImDrawList* dl = ImGui::GetForegroundDrawList();
        float W = static_cast<float>(mRenderData.rdWidth);
        float H = static_cast<float>(mRenderData.rdHeight);

        ImU32 blackCol = IM_COL32(0, 0, 0, static_cast<int>(mFadeAlpha * 255.0f));
        dl->AddRectFilled(ImVec2(0.0f, 0.0f), ImVec2(W, H), blackCol);

        bool showSpinner = (mFadeState == FadeState::LOADING_WAIT) ||
            (mFadeState == FadeState::FADE_IN && mFadeAlpha > 0.25f);

        if (showSpinner) {
            float cx = W * 0.5f;
            float cy = H * 0.5f;
            float radius = 28.0f;
            float thickness = 4.5f;

            const int totalSeg = 48;
            const int arcSeg = 36;
            float baseA = mLoadingSpinAngle;

            dl->PathClear();
            for (int i = 0; i <= arcSeg; ++i) {
                float a = baseA + (static_cast<float>(i) / static_cast<float>(totalSeg)) * 2.0f * IM_PI;
                dl->PathLineTo(ImVec2(cx + cosf(a) * radius, cy + sinf(a) * radius));
            }

            float spinAlpha = mFadeAlpha < 0.5f ? mFadeAlpha * 2.0f : 1.0f;
            dl->PathStroke(IM_COL32(255, 100, 200, static_cast<int>(spinAlpha * 230.0f)), false, thickness);

            const char* loadText = "Loading...";
            ImVec2 textSize = ImGui::CalcTextSize(loadText);
            dl->AddText(
                ImVec2(cx - textSize.x * 0.5f, cy + radius + 12.0f),
                IM_COL32(255, 100, 200, static_cast<int>(spinAlpha * 200.0f)),
                loadText
            );
        }
    }

    mRenderData.rdUIGenerateTime = mUIGenerateTimer.stop();
    mUIDrawTimer.start();
    mUserInterface.render();
    mRenderData.rdUIDrawTime = mUIDrawTimer.stop();

                if (pendingFadeOut) {
        mFadeState = FadeState::FADE_OUT;
        mFadeAlpha = 0.0f;
        Logger::log(1, "OGLRenderer: Menu action received, starting fade to black\n");
    }

    mLastTickTime = tickTime;

}
void OGLRenderer::ResetPlayerAfterDeath() {
    auto& enty = mGltfInstances.at(1);
    btRigidBody* body = enty->getRigidBody();

        btTransform spawnTrans;
    spawnTrans.setIdentity();
    spawnTrans.setOrigin(btVector3(0.0f, 2.0f, 0.0f));

    body->setWorldTransform(spawnTrans);
    body->getMotionState()->setWorldTransform(spawnTrans);
    body->setLinearVelocity(btVector3(0, 0, 0));
    body->setAngularVelocity(btVector3(0, 0, 0));
    body->setLinearFactor(btVector3(1.0f, 1.0f, 1.0f));       body->activate(true);

        mRenderData.rdPlayerHP = 100.0f;
    mRenderData.rdPlayerIsDead = false;

    ModelSettings settings = enty->getInstanceSettings();
    settings.msAnimClip = EntyAnim::Idle;
    settings.msCrossBlendDestAnimClip = EntyAnim::Idle;
    settings.msBlendingMode = blendMode::crossfade;
    settings.msAnimCrossBlendFactor = 1.0f;
    settings.msAnimTimePosition = 0.0f;
    settings.msPlayAnimation = true;
    enty->setInstanceSettings(settings);
    enty->checkForUpdates();

    mGameState = GameState::GAMEPLAY;

    Logger::log(1, "OGLRenderer: Player respawned\n");
}
void OGLRenderer::updatePlayerCharacter(float deltaTime) {
            if (mRenderData.rdHitStopTimer > 0.0f) {
        mRenderData.rdHitStopTimer -= deltaTime;
        return;     }
    auto& enty = mGltfInstances.at(1);
    if (!enty->getRigidBody()) return;

    btRigidBody* body = enty->getRigidBody();
    body->activate(true);
    if (mRenderData.rdPlayerIsDead) {
        static float deathTimer = 0.0f;
        float deathDuration = enty->getAnimationEndTime(EntyAnim::Death);

                if (deathTimer < deathDuration) {
            deathTimer += deltaTime;
        }

        ModelSettings settings = enty->getInstanceSettings();

                if (settings.msAnimClip != EntyAnim::Death) {
            settings.msAnimClip = EntyAnim::Death;
            settings.msCrossBlendDestAnimClip = EntyAnim::Death;
            settings.msBlendingMode = blendMode::crossfade;
            settings.msAnimCrossBlendFactor = 1.0f;
        }

        settings.msPlayAnimation = false;                 settings.msAnimTimePosition = std::min(deathTimer, deathDuration - 0.01f);

        enty->setInstanceSettings(settings);
        enty->checkForUpdates();

                        body->setLinearFactor(btVector3(0.0f, 1.0f, 0.0f));
        body->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));

        return;     }
    body->setAngularFactor(btVector3(0, 1, 0));

        static float dashDurationTimer = 0.0f;
    static float dashCooldownTimer = 0.0f;

        static float attackDurationTimer = 0.0f;     static int currentAttackAnim = EntyAnim::Idle;     static float attackTimeAccumulator = 0.0f;     static bool hasDealtDamage = false;

    if (dashDurationTimer > 0.0f) dashDurationTimer -= deltaTime;
    if (dashCooldownTimer > 0.0f) dashCooldownTimer -= deltaTime;

    if (attackDurationTimer > 0.0f) {
        attackDurationTimer -= deltaTime;
        attackTimeAccumulator += deltaTime; 
                                if (currentAttackAnim == EntyAnim::KICK) {
                        if (attackTimeAccumulator >= 0.483f && !hasDealtDamage) {
                std::cout << "[BOOM] Нога Энти вылетела! Запускаем хитчек!" << std::endl;

                                CheckPlayerMeleeHit(2.2f, 50.0f);

                hasDealtDamage = true;             }
        }
        if (currentAttackAnim == EntyAnim::Boxing) {
                        if (attackTimeAccumulator >= 0.433f && !hasDealtDamage) {
                std::cout << "[BOOM] Рука Энти вылетела! Запускаем хитчек!" << std::endl;

                                CheckPlayerMeleeHit(2.2f, 80.0f);

                hasDealtDamage = true;             }
        }
        if (currentAttackAnim == EntyAnim::FlyKick) {
                        if (attackTimeAccumulator >= 0.333f && !hasDealtDamage) {
                std::cout << "[BOOM] Рука Энти вылетела! Запускаем хитчек!" << std::endl;

                                CheckPlayerMeleeHit(12.0f, 20.0f);

                hasDealtDamage = true;             }
        }
        if (currentAttackAnim == EntyAnim::DoubleJumpKick) {
            if (attackTimeAccumulator >= 1.833f && !hasDealtDamage) {
                std::cout << "[BOOM] Рука Энти вылетела! Запускаем хитчек!" << std::endl;

                float baseDamage = 100.0f;
                bool isCrit = critDist(rngMeleeCrit) < 0.8f;                 float finalDamage = isCrit ? baseDamage * 2.0f : baseDamage;

                if (isCrit) {
                    std::cout << "[CRIT] Двойной урон!" << std::endl;
                }

                CheckPlayerMeleeHit(4.0f, finalDamage);
                hasDealtDamage = true;
            }
        }
    }
    

        bool keyC = glfwGetKey(mRenderData.rdWindow, GLFW_KEY_C) == GLFW_PRESS;
    bool keyG = glfwGetKey(mRenderData.rdWindow, GLFW_KEY_G) == GLFW_PRESS;
    bool keyE = glfwGetKey(mRenderData.rdWindow, GLFW_KEY_E) == GLFW_PRESS;
    bool keyZ = glfwGetKey(mRenderData.rdWindow, GLFW_KEY_Z) == GLFW_PRESS;
    bool keyH = glfwGetKey(mRenderData.rdWindow, GLFW_KEY_H) == GLFW_PRESS;
    bool keySpace = glfwGetKey(mRenderData.rdWindow, GLFW_KEY_SPACE) == GLFW_PRESS;
    bool mouseLMB = glfwGetMouseButton(mRenderData.rdWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    bool keyF = glfwGetKey(mRenderData.rdWindow, GLFW_KEY_F) == GLFW_PRESS;

    bool isSprinting = (std::abs(mRenderData.rdMoveForward) > 1 || std::abs(mRenderData.rdMoveRight) > 1);
    float maxSpeed = isSprinting ? 15.0f : 5.0f;

    ModelSettings settings = enty->getInstanceSettings();
    bool settingsChanged = false;
    float azimRad = glm::radians(mRenderData.rdViewAzimuth);

    glm::vec3 camForward = glm::normalize(glm::vec3(glm::sin(azimRad), 0.0f, -glm::cos(azimRad)));
    glm::vec3 camRight = glm::normalize(glm::cross(camForward, glm::vec3(0, 1, 0)));

    glm::vec3 moveDir(0.0f);
    if (mRenderData.rdMoveForward != 0) moveDir += camForward * (float)mRenderData.rdMoveForward;
    if (mRenderData.rdMoveRight != 0)   moveDir += camRight * (float)mRenderData.rdMoveRight;

    btVector3 currentVel = body->getLinearVelocity();


        bool isOnGround = false;
    btTransform currentTransform;
    body->getMotionState()->getWorldTransform(currentTransform);
    btVector3 rayStart = currentTransform.getOrigin();

    btVector3 rayEnd = rayStart + btVector3(0.0f, -1.4f, 0.0f);
    btCollisionWorld::ClosestRayResultCallback rayCallback(rayStart, rayEnd);
    mDynamicsWorld->rayTest(rayStart, rayEnd, rayCallback);

    if (rayCallback.hasHit() && rayCallback.m_collisionObject != body) {
        isOnGround = true;
    }


        if (keyE && isOnGround && dashDurationTimer <= 0.0f && dashCooldownTimer <= 0.0f && attackDurationTimer <= 0.0f) {
        dashDurationTimer = 0.3f;
        dashCooldownTimer = 1.5f;
    }

        if (isOnGround && attackDurationTimer <= 0.0f && dashDurationTimer <= 0.0f) {
        if (keyF) {
            currentAttackAnim = EntyAnim::FlyKick;
            attackDurationTimer = enty->getAnimationEndTime(currentAttackAnim);
            attackTimeAccumulator = 0.0f; 
            hasDealtDamage = false;
        }
        else if (mouseLMB) {
            currentAttackAnim = EntyAnim::Boxing;
            attackDurationTimer = enty->getAnimationEndTime(currentAttackAnim);
            attackTimeAccumulator = 0.0f;

            hasDealtDamage = false;
        }
        else if (keyG) {
            currentAttackAnim = EntyAnim::DoubleJumpKick;
            attackDurationTimer = enty->getAnimationEndTime(currentAttackAnim);
            attackTimeAccumulator = 0.0f;

            hasDealtDamage = false;
        }
        else if (keyC) {
            currentAttackAnim = EntyAnim::KICK;
            attackDurationTimer = enty->getAnimationEndTime(currentAttackAnim);
            attackTimeAccumulator = 0.0f;

            hasDealtDamage = false;
        }
    }

    if (mIsEntyStunned) {
        mEntyStunTimer += deltaTime;
        if (mEntyStunTimer >= mEntyStunDuration) {
            mIsEntyStunned = false;
                        auto entySettings = enty->getInstanceSettings();
            entySettings.msAnimClip = EntyAnim::Idle;
            entySettings.msPlayAnimation = true;
            enty->setInstanceSettings(entySettings);
        }
    }

    for (auto& enemy : mEnemies) {
        if (!enemy) continue;

                if (mRenderData.rdPlayerIsDead) continue;

        if (enemy->consumeHitPlayerFlag() && !mIsEntyStunned) {
                        mRenderData.rdPlayerHP -= 25.0f;             if (mRenderData.rdPlayerHP <= 0.0f) mRenderData.rdPlayerHP = 0.0f;
                        auto entySettings = enty->getInstanceSettings();
            if (mRenderData.rdPlayerHP <= 0.0f) {
                mRenderData.rdPlayerIsDead = true;
                std::cout << "[GAME OVER] Энти погибла!" << std::endl;

                break;             }
            else {
                entySettings.msAnimClip = EntyAnim::HitReaction;                 entySettings.msCrossBlendDestAnimClip = EntyAnim::HitReaction;
                entySettings.msBlendingMode = blendMode::crossfade;
                entySettings.msAnimCrossBlendFactor = 1.0f;
                entySettings.msAnimTimePosition = 0.0f;
                entySettings.msPlayAnimation = true;
                entySettings.msAnimSpeed = 1.0f;

                enty->setInstanceSettings(entySettings);
                enty->checkForUpdates();

                                mIsEntyStunned = true;
                mEntyStunTimer = 0.0f;

                                attackDurationTimer = 0.0f;
                attackTimeAccumulator = 0.0f;
                hasDealtDamage = false;
            }
        }
    }
        int currentTargetAnim = EntyAnim::Idle;

    if (mIsEntyStunned) {
        currentTargetAnim = EntyAnim::HitReaction;
    }
    else if (dashDurationTimer > 0.0f) {
        currentTargetAnim = EntyAnim::DashBack;
    }
        else if (attackDurationTimer > 0.0f) {
        currentTargetAnim = currentAttackAnim;
    }
        else if (!isOnGround) {
        currentTargetAnim = EntyAnim::Jump;
    }
    else if (keyZ) {
        currentTargetAnim = EntyAnim::Dance1;
    }
    else if (keyH) {
        currentTargetAnim = EntyAnim::DanceWin;
    }
    else if (glm::length(moveDir) > 0.1f) {
        currentTargetAnim = isSprinting ? EntyAnim::Run : EntyAnim::Walk;
    }
    else {
        currentTargetAnim = EntyAnim::Idle;
    }


        bool isMovementBlocked = (
        mIsEntyStunned ||
        attackDurationTimer > 0.0f ||         currentTargetAnim == EntyAnim::Dance1 ||
        currentTargetAnim == EntyAnim::DanceWin
        );

    bool isDashing = (currentTargetAnim == EntyAnim::DashBack);

    if (isMovementBlocked || isDashing) {
        moveDir = glm::vec3(0.0f);
    }


        if (isDashing) {
        btTransform trans;
        body->getMotionState()->getWorldTransform(trans);

        btVector3 backwardDir = quatRotate(trans.getRotation(), btVector3(0, 0, -1));

        float dashSpeed = 25.0f;
        float dashAccel = 30.0f * deltaTime;

        btVector3 targetVel(backwardDir.x() * dashSpeed, currentVel.y(), backwardDir.z() * dashSpeed);
        btVector3 finalVel = currentVel.lerp(targetVel, dashAccel);
        body->setLinearVelocity(finalVel);

        body->setAngularVelocity(btVector3(0, 0, 0));
    }
        else if (attackDurationTimer > 0.0f && (currentAttackAnim == EntyAnim::FlyKick || currentAttackAnim == EntyAnim::DoubleJumpKick)) {
        btTransform trans;
        body->getMotionState()->getWorldTransform(trans);

                btVector3 forwardDir = quatRotate(trans.getRotation(), btVector3(0, 0, 1));

        float attackMoveSpeed = 0.0f; 
                if (currentAttackAnim == EntyAnim::FlyKick) {
                                    if (attackTimeAccumulator > 0.0f && attackTimeAccumulator < 1.2f) {
                attackMoveSpeed = 20.0f;             }
        }
                else if (currentAttackAnim == EntyAnim::DoubleJumpKick) {
                        if (attackTimeAccumulator > 0.8f && attackTimeAccumulator < 2.4f) {
                attackMoveSpeed = 5.0f;
            }
      
        }

        float accel = 25.0f * deltaTime;
        btVector3 targetVel(forwardDir.x() * attackMoveSpeed, currentVel.y(), forwardDir.z() * attackMoveSpeed);
        btVector3 finalVel = currentVel.lerp(targetVel, accel);
        body->setLinearVelocity(finalVel);
        body->setAngularVelocity(btVector3(0, 0, 0));
    }
        else if (glm::length(moveDir) > 0.1f && !isMovementBlocked) {
        moveDir = glm::normalize(moveDir);
        float accel = 10.0f * deltaTime;

        btVector3 targetVel(moveDir.x * maxSpeed, currentVel.y(), moveDir.z * maxSpeed);
        btVector3 finalVel = currentVel.lerp(targetVel, accel);
        body->setLinearVelocity(finalVel);

        float targetAngle = atan2(moveDir.x, moveDir.z);
        btTransform trans;
        body->getMotionState()->getWorldTransform(trans);

        btVector3 currentForward = quatRotate(trans.getRotation(), btVector3(0, 0, 1));
        float currentAngle = atan2(currentForward.x(), currentForward.z());
        float angleDiff = targetAngle - currentAngle;

        while (angleDiff > M_PI) angleDiff -= 2.0f * M_PI;
        while (angleDiff < -M_PI) angleDiff += 2.0f * M_PI;

        body->setAngularVelocity(btVector3(0, angleDiff * 12.0f, 0));
    }
        else {
        float decel = 15.0f * deltaTime;
        btVector3 targetVel(0.0f, currentVel.y(), 0.0f);
        btVector3 finalVel = currentVel.lerp(targetVel, decel);
        body->setLinearVelocity(finalVel);
        body->setAngularVelocity(btVector3(0, 0, 0));
    }


        if (keySpace && !mWasSpacePressed && isOnGround && !isMovementBlocked && !isDashing) {
        float jumpForce = 340.0f;
        body->applyCentralImpulse(btVector3(0.0f, jumpForce, 0.0f));
        isOnGround = false;
    }
    mWasSpacePressed = keySpace;


        if (currentTargetAnim != settings.msCrossBlendDestAnimClip) {
        settings.msCrossBlendDestAnimClip = currentTargetAnim;
        settings.msAnimClip = currentTargetAnim;

        settings.msAnimCrossBlendFactor = 1.0f;
        settings.msBlendingMode = blendMode::crossfade;

        settings.msAnimTimePosition = 0.0f;
        settingsChanged = true;
    }

        bool forceManualUpdate = false;

        if (attackDurationTimer > 0.0f) {
        settings.msPlayAnimation = false;         settings.msAnimTimePosition = attackTimeAccumulator;         forceManualUpdate = true;     }
    else {
        settings.msPlayAnimation = true;     }

    if (settingsChanged || forceManualUpdate) {
        if (currentTargetAnim == EntyAnim::DashBack) {
            settings.msAnimSpeed = 1.4f;
        }
        else {
            settings.msAnimSpeed = 1.0f;
        }

        enty->setInstanceSettings(settings);
        enty->checkForUpdates();
    }
}

void OGLRenderer::CheckPlayerMeleeHit(float hitRadius, float damage) {
        btTransform playerTrans;
    mGltfInstances.at(1)->getRigidBody()->getMotionState()->getWorldTransform(playerTrans);
    glm::vec3 playerPos(playerTrans.getOrigin().x(), playerTrans.getOrigin().y(), playerTrans.getOrigin().z());

        btVector3 btForward = quatRotate(playerTrans.getRotation(), btVector3(0, 0, 1));
    glm::vec3 playerForward = glm::normalize(glm::vec3(btForward.x(), 0.0f, btForward.z()));

        for (auto& enemy : mEnemies) {
        if (!enemy) continue;

                btTransform enemyTrans;
        enemy->getBody()->getMotionState()->getWorldTransform(enemyTrans);
        glm::vec3 enemyPos(enemyTrans.getOrigin().x(), enemyTrans.getOrigin().y(), enemyTrans.getOrigin().z());

                glm::vec3 dirToEnemy = enemyPos - playerPos;

                        dirToEnemy.y = 0.0f;

                float distance = glm::length(dirToEnemy);

                if (distance <= hitRadius) {
            if (distance > 0.01f) dirToEnemy = glm::normalize(dirToEnemy);

                        float dotProduct = glm::dot(playerForward, dirToEnemy);

            if (dotProduct > 0.5f) {                 std::cout << "[HIT] Попадание! Дистанция до врага: " << distance << std::endl;

                                enemy->TakeDamage(damage);
                if (enemy->isDead()) {
                    gAudio.playSound("assets/audio/death_punch.wav");
                                                                                mRenderData.rdHitStopTimer = 0.15f;                      mRenderData.rdShakeDuration = 0.15f;                     mRenderData.rdShakeIntensity = 0.03f;
                    mRenderData.rdZoomTimer = 0.25f;                      }
                else {
                    gAudio.playSound("assets/audio/punch.MP3");
                                                                                mRenderData.rdHitStopTimer = 0.06f;
                    mRenderData.rdShakeDuration = 0.10f;
                    mRenderData.rdShakeIntensity = 0.04f;
                    mRenderData.rdZoomTimer = 0.0f;                      }
            }
        }
    }
}
void OGLRenderer::cleanup() {
    if (mDynamicsWorld) {
        for (int i = mDynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
            btCollisionObject* obj = mDynamicsWorld->getCollisionObjectArray()[i];
            mDynamicsWorld->removeCollisionObject(obj);
            // Не удаляй obj здесь вручную, если motion state/shape принадлежат instance'ам
            // Лучше оставить удаление shapes/bodies на сторону GltfInstance
        }
        delete mDynamicsWorld;
        delete mSolver;
        delete mOverlappingPairCache;
        delete mDispatcher;
        delete mCollisionConfiguration;
        mDynamicsWorld = nullptr;
    }

    // Безопасная очистка моделей
    for (auto& model : mGltfModels) {
        if (model) {
            model->cleanup();
            model.reset();
        }
    }
    mGltfModels.clear();

    // Инстансы тоже нужно почистить (если они держат rigid body и т.д.)
    for (auto& inst : mGltfInstances) {
        if (inst) {
            // если есть cleanup у instance — вызови
            inst.reset();
        }
    }
    mGltfInstances.clear();

    if (mMenuController) {
        mMenuController->cleanup();
        mMenuController.reset();
    }

    // NJ menu model
    if (mNJMenuModel) {
        mNJMenuModel->cleanup();
        mNJMenuModel.reset();
    }

    mGltfGPUDualQuatShader.cleanup();
    mGltfGPUShader.cleanup();
    mGltfGPUShaderToy.cleanup();
    mGltfGPUShaderToyChannelPack.cleanup();
    mGltfGPUDualQuatChannelShader.cleanup();
    mStaticShader.cleanup();
    mLineShader.cleanup();
    mBloomBlurShader.cleanup();
    mBloomCompositeShader.cleanup();
    mDeathGlowShader.cleanup();
    mDeathGlowCompositeShader.cleanup();

    mUserInterface.cleanup();
    mVertexBuffer.cleanup();
    mGltfShaderStorageBuffer.cleanup();
    mGltfDualQuatSSBuffer.cleanup();
    mUniformBuffer.cleanup();

    mFramebuffer.cleanup();
    mMSFramebuffer.cleanup();
    mBlurFBO[0].cleanup();
    mBlurFBO[1].cleanup();
    mDeathCaptureFBO.cleanup();

    if (mBloomVAO != 0) {
        glDeleteVertexArrays(1, &mBloomVAO);
        mBloomVAO = 0;
    }

    mSkyBox.cleanup();
    gAudio.shutdown();
}

