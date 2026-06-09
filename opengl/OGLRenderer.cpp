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
#include <iostream>

OGLRenderer::OGLRenderer(GLFWwindow *window) {
  mRenderData.rdWindow = window;
}

bool OGLRenderer::init(unsigned int width, unsigned int height) {
  /* randomize rand() */
  std::srand(static_cast<int>(time(NULL)));

  /* required for perspective */
  mRenderData.rdWidth = width;
  mRenderData.rdHeight = height;

  /* initalize GLAD */
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    Logger::log(1, "%s error: failed to initialize GLAD\n", __FUNCTION__);
    return false;
  }

  mCollisionConfiguration = new btDefaultCollisionConfiguration();
  mDispatcher = new btCollisionDispatcher(mCollisionConfiguration);
  mOverlappingPairCache = new btDbvtBroadphase(); // Для быстрого отсечения столкновений
  mSolver = new btSequentialImpulseConstraintSolver();
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
      glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // Ошибки будут прилетать сразу в момент вызова
      glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity,
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
  Logger::log(1, "%s: framebuffer succesfully initialized\n", __FUNCTION__);

  mVertexBuffer.init();
  Logger::log(1, "%s: vertex buffer successfully created\n", __FUNCTION__);

  size_t uniformMatrixBufferSize = 2 * sizeof(glm::mat4);
  mUniformBuffer.init(uniformMatrixBufferSize);
  Logger::log(1, "%s: matrix uniform buffer (size %i bytes) successfully created\n", __FUNCTION__, uniformMatrixBufferSize);

  if (!mLineShader.loadShaders("shader/line.vert", "shader/line.frag")) {
    Logger::log(1, "%s: line shader loading failed\n", __FUNCTION__);
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

  /* add backface culling and depth test already here */
  glDisable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glLineWidth(3.0);

  /* load 2x the woman and the sq model */
 // test 12.01.26 - 1 model only 1 instance
  //mGltfModels.resize(3);
  
  mGltfModels.resize(9);
  mGltfModels.at(0) = std::make_shared<GltfModel>();
  std::string modelFilename = "assets/StonehengeMay.gltf";
  std::string modelTexFilename = "textures/StoneMay.png";

  /* 11.04.26 нужно добавить сшиватель для roughness and metallic а то там только одно перемнная на двоих ну и шейдер доделать
  if (!mGltfModels.at(0)->loadModel(mRenderData, modelFilename, modelTexFilename, normalTex, roughTex)) {
      Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename.c_str());
      return false;
  }
  */
  if (!mGltfModels.at(0)->loadModel(mRenderData, modelFilename, modelTexFilename)) {
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
  std::string modelFilename2 = "assets/EntyMay.gltf";
  std::string modelTexFilename2 = "textures/EntyMay.png";
  if (!mGltfModels.at(1)->loadModel(mRenderData, modelFilename2, modelTexFilename2)) {
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
  std::string modelFilename4 = "assets/vorota ishtarMay.gltf";
  std::string modelTexFilename4 = "textures/vorota.png";
  if (!mGltfModels.at(3)->loadModel(mRenderData, modelFilename4, modelTexFilename4)) {
      Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename4.c_str());
      return false;
  }

  mGltfModels.at(4) = std::make_shared<GltfModel>();
  std::string modelFilename5 = "assets/floor vorot ishtar.gltf";
  std::string modelTexFilename5 = "textures/floorVrat.png";
  if (!mGltfModels.at(4)->loadModel(mRenderData, modelFilename5, modelTexFilename5)) {
      Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename5.c_str());
      return false;
  }
 
  mGltfModels.at(5) = std::make_shared<GltfModel>();
  std::string modelFilename6 = "assets/kanalMay.gltf";
  std::string modelTexFilename6 = "textures/Stock.png";
  if (!mGltfModels.at(5)->loadModel(mRenderData, modelFilename6, modelTexFilename6)) {
      Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename6.c_str());
      return false;
  }

  mGltfModels.at(6) = std::make_shared<GltfModel>();
  std::string modelFilename7 = "assets/gates 1.0.gltf";
  std::string modelTexFilename7 = "textures/gates.png";
  if (!mGltfModels.at(6)->loadModel(mRenderData, modelFilename7, modelTexFilename7)) {
      Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename7.c_str());
      return false;
  }

  mGltfModels.at(7) = std::make_shared<GltfModel>();
  std::string modelFilename8 = "assets/Arena.gltf";
  std::string modelTexFilename8 = "textures/Arena.png";
  if (!mGltfModels.at(7)->loadModel(mRenderData, modelFilename8, modelTexFilename8)) {
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
  //mGltfModels.at(0)->uploadVertexBuffers();
  //mGltfModels.at(0)->uploadIndexBuffer();
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

  /* create glTF instances from the model */
  /*
  for (int i = 0; i < 5; ++i) {
    int xPos = std::rand() % 40 - 20;
    int zPos = std::rand() % 40 - 20;
    int modelNo = std::rand() % 2;
    mGltfInstances.emplace_back(std::make_shared<GltfInstance>(mGltfModels.at(modelNo), glm::vec2(static_cast<float>(xPos),
      static_cast<float>(zPos)), true));
    numTriangles += mGltfModels.at(modelNo)->getTriangleCount();
  }
  // dq model
  
  for (int i = 0; i < 10; ++i) {
    int xPos = std::rand() % 50 - 25;
    int zPos = std::rand() % 20 - 50;
    mGltfInstances.emplace_back(std::make_shared<GltfInstance>(mGltfModels.at(2), glm::vec2(static_cast<float>(xPos),
      static_cast<float>(zPos)), true));
    numTriangles += mGltfModels.at(2)->getTriangleCount();
  }
  */
  // 1 model coordinates
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

  int xPos8 = 392;
  int yPos8 = -1;
  int zPos8 = 322;
  glm::vec3 CustomRot3 = glm::vec3(0.0f, 90.0f, 0.0f);
  mGltfInstances.emplace_back(std::make_shared<GltfInstance>(mGltfModels.at(7), glm::vec3(static_cast<float>(xPos8), static_cast<float>(yPos8),
      static_cast<float>(zPos8)), CustomRot3));
  numTriangles += static_cast<unsigned int>(mGltfModels.at(7)->getTriangleCount());

  int xPos9 = 5;
  int yPos9 = 3;
  int zPos9 = 0;
  mGltfInstances.emplace_back(std::make_shared<GltfInstance>(mGltfModels.at(8), glm::vec3(static_cast<float>(xPos9), static_cast<float>(yPos9),
      static_cast<float>(zPos9)), defaultRot));
  numTriangles += static_cast<unsigned int>(mGltfModels.at(8)->getTriangleCount());

  addPhysicsToStonehenge(mGltfInstances.at(0), mDynamicsWorld, glm::vec3(77, 0, 580),defaultRot);
  addPhysicsToEnty(mGltfInstances.at(1), mDynamicsWorld, glm::vec3(0, 0, 0)); // Она заспавнится на высоте 10м и упадет на пол
  {
      ModelSettings entySettings = mGltfInstances.at(1)->getInstanceSettings();
      entySettings.msAnimClip = EntyAnim::Idle; 
      entySettings.msPlayAnimation = true;      
      mGltfInstances.at(1)->setInstanceSettings(entySettings);
  }
  addPhysicsToStonehenge(mGltfInstances.at(4), mDynamicsWorld, glm::vec3(0, 0, 180),defaultRot);
  addPhysicsToStonehenge(mGltfInstances.at(2), mDynamicsWorld, glm::vec3(50, 0.1, 419),CustomRot);
  addPhysicsToStonehenge(mGltfInstances.at(5), mDynamicsWorld, glm::vec3(-166, 1, 261), CustomRot2);
  addPhysicsToStonehenge(mGltfInstances.at(7), mDynamicsWorld, glm::vec3(392, -1, 322), CustomRot3);

  addPhysicsToEnemy(mGltfInstances.at(8), mDynamicsWorld, glm::vec3(5, 3, 0)); 

  auto enemyAgent = std::make_unique<Enemy>(mGltfInstances.at(8), mGltfInstances.at(8)->getRigidBody());
  mEnemies.push_back(std::move(enemyAgent));

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
  // проверка от 10.01.26
  printf("\n--- [SSBO DEBUG] ---\n");
  printf("Total Joint Matrices: %d\n", jointMatrixSize);
  printf("Required Matrix Buffer Size: %llu bytes\n", (unsigned long long)modelJointMatrixBufferSize);
  printf("Required DualQuat Buffer Size: %llu bytes\n", (unsigned long long)modelJointDualQuatBufferSize);

  // Проверим лимит видеокарты
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

  // конец проверки
  mGltfShaderStorageBuffer.init(modelJointMatrixBufferSize);
  Logger::log(1, "%s: glTF joint matrix shader storage buffer (size %i bytes) successfully created\n", __FUNCTION__, modelJointMatrixBufferSize);
  
  mGltfDualQuatSSBuffer.init(modelJointDualQuatBufferSize);
  Logger::log(1, "%s: glTF joint dual quaternions shader storage buffer (size %i bytes) successfully created\n", __FUNCTION__, modelJointDualQuatBufferSize);

  /* valid, but emtpy */
  mLineMesh = std::make_shared<OGLMesh>();
  Logger::log(1, "%s: line mesh storage initialized\n", __FUNCTION__);

  mFrameTimer.start();
  Logger::log(1, "%s: Starting to load instances...\n", __FUNCTION__);
  return true;
}

void OGLRenderer::setSize(unsigned int width, unsigned int height) {
  /* handle minimize */
  if (width == 0 || height == 0) {
    return;
  }

  mRenderData.rdWidth = width;
  mRenderData.rdHeight = height;

  mFramebuffer.resize(width, height);
  glViewport(0, 0, width, height);

  Logger::log(1, "%s: resized window to %dx%d\n", __FUNCTION__, width, height);
}

void OGLRenderer::uploadData(OGLMesh vertexData) {
  mVertexBuffer.uploadData(vertexData);
}

void OGLRenderer::handleKeyEvents(int key, int scancode, int action, int mods) {
    if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        mShowMenu = !mShowMenu;
        Logger::log(1, "%s: Menu state toggled: %s\n", __FUNCTION__, mShowMenu ? "ON" : "OFF");
        if (mShowMenu) {
            glfwSetInputMode(mRenderData.rdWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else {
            // Если у тебя камера управляется правой кнопкой (mMouseLock), 
            // решай сам — скрывать курсор обратно или нет. Пока оставим так:
            glfwSetInputMode(mRenderData.rdWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

void OGLRenderer::handleMouseButtonEvents(int button, int action, int mods) {
  /* forward to ImGui */
  ImGuiIO& io = ImGui::GetIO();
  if (button >= 0 && button < ImGuiMouseButton_COUNT) {
    io.AddMouseButtonEvent(button, action == GLFW_PRESS);
  }

  /* hide from application if above ImGui window */
  if (io.WantCaptureMouse) {
    return;
  }
  
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
  //if (button == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    mMouseLock = !mMouseLock;
    if (mMouseLock) {
      glfwSetInputMode(mRenderData.rdWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      /* enable raw mode if possible */
      if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(mRenderData.rdWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
      }
    } else {
      glfwSetInputMode(mRenderData.rdWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
  }
}

void OGLRenderer::handleMousePositionEvents(double xPos, double yPos) {
  /* forward to ImGui */
  ImGuiIO& io = ImGui::GetIO();
  io.AddMousePosEvent((float)xPos, (float)yPos);

  /* hide from application if above ImGui window */
  if (io.WantCaptureMouse) {
    return;
  }
  if (mShowMenu) {
      return;
  }
  /* calculate relative movement from last position */
  int mouseMoveRelX = static_cast<int>(xPos) - mMouseXPos;
  int mouseMoveRelY = static_cast<int>(yPos) - mMouseYPos;

  if (mMouseLock) {
    mRenderData.rdViewAzimuth += mouseMoveRelX / 10.0;
    /* keep between 0 and 360 degree */
    if (mRenderData.rdViewAzimuth < 0.0) {
      mRenderData.rdViewAzimuth += 360.0;
    }
    if (mRenderData.rdViewAzimuth >= 360.0) {
      mRenderData.rdViewAzimuth -= 360.0;
    }

    mRenderData.rdViewElevation -= mouseMoveRelY / 10.0;
    /* keep between -89 and +89 degree */
    if (mRenderData.rdViewElevation > 89.0) {
      mRenderData.rdViewElevation = 89.0;
    }
    if (mRenderData.rdViewElevation < -89.0) {
      mRenderData.rdViewElevation = -89.0;
    }
  }

  /* save old values*/
  mMouseXPos = static_cast<int>(xPos);
  mMouseYPos = static_cast<int>(yPos);
}

void OGLRenderer::addPhysicsToEnty(std::shared_ptr<GltfInstance> entyInstance, btDiscreteDynamicsWorld* dynamicsWorld, glm::vec3 spawnPos) {
    // РЕАЛЬНЫЕ РАЗМЕРЫ: Радиус 0.3м, Высота цилиндра 1.2м. 
    // Итого: 1.8 + 0.3 + 0.3 = 2.4 метра общей высоты!
    btCollisionShape* colShape = new btCapsuleShape(0.3f, 1.8f);

    btTransform startTransform;
    startTransform.setIdentity();
    // Спавним на 10 метров вверх
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
    // РЕАЛЬНЫЕ РАЗМЕРЫ: Радиус 0.3м, Высота цилиндра 1.2м. 
    // Итого: 1.8 + 0.3 + 0.3 = 2.4 метра общей высоты!
    btCollisionShape* colShape = new btCapsuleShape(0.3f, 1.8f);

    btTransform startTransform;
    startTransform.setIdentity();
    // Спавним на 10 метров вверх
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
void OGLRenderer::addPhysicsToStonehenge(std::shared_ptr<GltfInstance> stonehengeInstance, btDiscreteDynamicsWorld* dynamicsWorld, glm::vec3 pos, glm::vec3 rot) {
    // Получаем ИДЕАЛЬНО точный физический шейп из glTF
    btCollisionShape* groundShape = stonehengeInstance->getModel()->getPhysicsShape();

    btTransform groundTransform;
    groundTransform.setIdentity();

    // ВНИМАНИЕ: Я убрал смещение pos.y - 0.5f, потому что теперь коллизия 
    // идеально совпадает с визуальной сеткой.
    groundTransform.setOrigin(btVector3(pos.x, pos.y, pos.z));

    // 3. МАТЕМАТИКА ПОВОРОТА ДЛЯ BULLET
    // Создаем кватернион Bullet и переводим твои градусы из Медиатора в радианы.
    // Порядок углов в Bullet: Yaw (Y), Pitch (X), Roll (Z)
    btQuaternion btRot;
    btRot.setEuler(
        glm::radians(rot.y), // Вращение вокруг Y
        glm::radians(rot.x), // Вращение вокруг X
        glm::radians(rot.z)  // Вращение вокруг Z
    );
    groundTransform.setRotation(btRot);

    // Для статичных объектов масса ВСЕГДА = 0.
    btScalar mass(0.0f);
    btVector3 localInertia(0, 0, 0);

    btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
    btRigidBody* body = new btRigidBody(rbInfo);

    // Делаем камни шероховатыми
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

  /* speed up movement with shift */
  if ((glfwGetKey(mRenderData.rdWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ||
      (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)) {
    mRenderData.rdMoveForward *= 4;
    mRenderData.rdMoveRight *= 4;
    mRenderData.rdMoveUp *= 4;
  }
}

void OGLRenderer::draw() {
  /* handle minimize */
  while (mRenderData.rdWidth == 0 || mRenderData.rdHeight == 0) {
    glfwGetFramebufferSize(mRenderData.rdWindow, &mRenderData.rdWidth, &mRenderData.rdHeight);
    glfwWaitEvents();
  }

  /* get time difference for movement */
  /* get time difference for movement */
  double tickTime = glfwGetTime();
  mRenderData.rdTickDiff = tickTime - mLastTickTime;

  // ИСПОЛЬЗУЕМ rdTickDiff (он гарантированно в секундах)
  float deltaTime = static_cast<float>(mRenderData.rdTickDiff);

  // Защита от провалов под текстуры при долгих загрузках кадров
  if (deltaTime > 0.1f) {
      deltaTime = 0.1f;
  }

  // ШАГ 1: СИМУЛЯЦИЯ ФИЗИКИ
  if (mDynamicsWorld) {
      mDynamicsWorld->stepSimulation(deltaTime, 10);
  }

  // ШАГ 2: СИНХРОНИЗАЦИЯ ФИЗИКИ С ГРАФИКОЙ
  for (auto& instance : mGltfInstances) {
      instance->syncPhysics();
  }

  mRenderData.rdFrameTime = mFrameTimer.stop();
  mFrameTimer.start();

  handleMovementKeys();

  updatePlayerCharacter(deltaTime);

  btTransform entyTrans;
  mGltfInstances.at(1)->getRigidBody()->getMotionState()->getWorldTransform(entyTrans);
  glm::vec3 entyPos(entyTrans.getOrigin().x(), entyTrans.getOrigin().y(), entyTrans.getOrigin().z());

  // Говорим всем врагам обновиться и передаем им координаты цели
  for (auto& enemy : mEnemies) {
      enemy->update(deltaTime, entyPos);
  }
  /* draw to framebuffer */
  mFramebuffer.bind();

  glClearColor(0.52f, 0.84f, 1.0f, 0.81f);
  glClearDepth(1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  mMatrixGenerateTimer.start();
  mProjectionMatrix = glm::perspective(
    glm::radians(static_cast<float>(mRenderData.rdFieldOfView)),
    static_cast<float>(mRenderData.rdWidth) / static_cast<float>(mRenderData.rdHeight),
    0.01f, 500.0f);

  glm::vec3 playerPos = mGltfInstances.at(1)->getWorldPosition();
  playerPos.y += 0.8f;
  mViewMatrix = mCamera.getThirdPersonViewMatrix(mRenderData, playerPos);

  /* animate and update inverse kinematics */
  mRenderData.rdIKTime = 0.0f;
  for (auto& instance : mGltfInstances) {
      if (instance->getJointMatrixSize() > 0) {
          instance->updateAnimation();

          mIKTimer.start();
          instance->solveIK();
          mRenderData.rdIKTime += static_cast<float>(mIKTimer.stop());
      }
  }

  /* save value to avoid changes during later call */
  int selectedInstance = mRenderData.rdCurrentSelectedInstance;
  glm::vec3 modelWorldPos = mGltfInstances.at(selectedInstance)->getWorldPosition();
  glm::quat modelWorldRot = mGltfInstances.at(selectedInstance)->getWorldRotation();

  mLineMesh->vertices.clear();

  /* get gltTF skeleton */
  mSkeletonLineIndexCount = 0;
  for (const auto &instance : mGltfInstances) {
    ModelSettings settings = instance->getInstanceSettings();
    if (settings.msDrawSkeleton && instance->getJointMatrixSize() > 0) {
      std::shared_ptr<OGLMesh> mesh = instance->getSkeleton();
      mSkeletonLineIndexCount += mesh->vertices.size();
      mLineMesh->vertices.insert(mLineMesh->vertices.begin(),
        mesh->vertices.begin(), mesh->vertices.end());
    }
  }
  
  /* get coordinate arrows for the IK target of current instance only */
  mCoordArrowsLineIndexCount = 0;
  {
      if (mGltfInstances.at(selectedInstance)->getJointMatrixSize() > 0) {
          ModelSettings ikSettings = mGltfInstances.at(selectedInstance)->getInstanceSettings();
          if (ikSettings.msIkMode == ikMode::ccd ||
              ikSettings.msIkMode == ikMode::fabrik) {
              mCoordArrowsMesh = mCoordArrowsModel.getVertexData();
              mCoordArrowsLineIndexCount += mCoordArrowsMesh.vertices.size();
              std::for_each(mCoordArrowsMesh.vertices.begin(), mCoordArrowsMesh.vertices.end(),
                  [=](auto& n) {
                      n.color /= 2.0f;
                      n.position = modelWorldRot * n.position;
                      n.position += ikSettings.msIkTargetWorldPos;
                  });

              mLineMesh->vertices.insert(mLineMesh->vertices.end(),
                  mCoordArrowsMesh.vertices.begin(), mCoordArrowsMesh.vertices.end());
          }
      }
  }
  /* draw coordiante arrows*/
  mCoordArrowsMesh = mCoordArrowsModel.getVertexData();
  mCoordArrowsLineIndexCount += mCoordArrowsMesh.vertices.size();
  std::for_each(mCoordArrowsMesh.vertices.begin(), mCoordArrowsMesh.vertices.end(),
    [=](auto &n){
      n.color /= 2.0f;
      n.position = modelWorldRot * n.position;
      n.position += modelWorldPos;
  });

  mLineMesh->vertices.insert(mLineMesh->vertices.end(),
    mCoordArrowsMesh.vertices.begin(), mCoordArrowsMesh.vertices.end());

  mRenderData.rdMatrixGenerateTime = mMatrixGenerateTimer.stop();

  mUploadToUBOTimer.start();
  std::vector<glm::mat4> matrixData;
  matrixData.push_back(mViewMatrix);
  matrixData.push_back(mProjectionMatrix);
  mUniformBuffer.uploadUboData(matrixData, 0);

  mModelJointMatrices.clear();
  mModelJointDualQuats.clear();
  mStaticInstances.clear();
  mGltfMatrixInstances.clear();
  mGltfDQInstances.clear();

  unsigned int numTriangles = 0;

  for (const auto &instance : mGltfInstances) {
    ModelSettings settings = instance->getInstanceSettings();
    if (!settings.msDrawModel) {
      continue;
    }

    // --- ВОТ ИЗМЕНЕНИЕ: Сначала проверяем, есть ли вообще кости ---
    if (instance->getJointMatrixSize() == 0) {
      mStaticInstances.emplace_back(instance); // Объект без костей (наш пол)
      numTriangles += static_cast<unsigned int>(instance->getModel()->getTriangleCount());
      continue;
    } 
    else if (settings.msVertexSkinningMode == skinningMode::dualQuat) {
      // Твой старый код для DQ...
      std::vector<glm::mat2x4> quats = instance->getJointDualQuats();
      mModelJointDualQuats.insert(mModelJointDualQuats.end(), quats.begin(), quats.end());
      size_t padding = 256 - quats.size();
      for (size_t i = 0; i < padding; ++i) { mModelJointDualQuats.push_back(glm::mat2x4(0.0f)); }
      mGltfDQInstances.emplace_back(instance);
    } 
    else {
      // Твой старый код для матриц...
      std::vector<glm::mat4> mats = instance->getJointMatrices();
      mModelJointMatrices.insert(mModelJointMatrices.end(), mats.begin(), mats.end());
      size_t padding = 256 - mats.size();
      for (size_t i = 0; i < padding; ++i) { mModelJointMatrices.push_back(glm::mat4(1.0f)); }
      mGltfMatrixInstances.emplace_back(instance);
    }
   numTriangles += static_cast<unsigned int>(instance->getModel()->getTriangleCount());
  }
 
 mRenderData.rdTriangleCount = numTriangles;

  mGltfShaderStorageBuffer.uploadSsboData(mModelJointMatrices, 1);
  mGltfDualQuatSSBuffer.uploadSsboData(mModelJointDualQuats, 2);

  mRenderData.rdUploadToUBOTime = mUploadToUBOTimer.stop();

  /* upload vertex data */
  mUploadToVBOTimer.start();

  uploadData(*mLineMesh);

  mRenderData.rdUploadToVBOTime = mUploadToVBOTimer.stop();

  /* draw the glTF models */

  // --- 1. СНАЧАЛА РИСУЕМ ПОЛ (СТАТИКУ) ---
  mStaticShader.use(); // Тот самый "ня статик" шейдер

  // Направление: свет падает сверху вниз под углом
  glm::vec3 sunDirection = glm::vec3(-0.3f, -1.0f, -0.5f);

  // Передаем в шейдер
  if (mStaticShader.getUniformLocation("u_LightDir")) {
      mStaticShader.setUniformValue(sunDirection);
  }

  mStaticShader.getUniformLocation("u_AlbedoMap");
  mStaticShader.setUniformValue(0);
  mStaticShader.getUniformLocation("u_NormalMap");
  mStaticShader.setUniformValue(1);
  mStaticShader.getUniformLocation("u_HasNormalMap");

  mStaticShader.getUniformLocation("u_MetRoughMap");
  mStaticShader.setUniformValue(2);
  
  for (const auto& instance : mStaticInstances) {
      instance->getRootNode()->calculateNodeMatrix();
      // 1. Получаем матрицу положения модели в мире
      glm::mat4 modelMatrix = instance->getRootNode()->getNodeMatrix();

      mStaticShader.getUniformLocation("u_HasNormalMap");
      mStaticShader.setUniformValue(instance->getModel()->mHasNormalMap ? 1 : 0);

      mStaticShader.getUniformLocation("u_HasMetRoughMap");
      mStaticShader.setUniformValue(instance->getModel()->mHasMetRoughMap ? 1 : 0);

      mStaticShader.getUniformLocation("u_Model");
       mStaticShader.setUniformValue(modelMatrix);
      
      instance->getModel()->draw();
  }

  unsigned int matrixPos = 0;
  int instanceCounter = 0; // Добавляем счетчик вручную
  
  mGltfGPUShader.use();
  //printf("Drawing %llu matrix instances\n", mGltfMatrixInstances.size()); 
  for (const auto &instance : mGltfMatrixInstances) {
    // set position inside the SSBO 
      if (matrixPos == 0) { // Чтобы не спамить, выведем только для первого
        //  printf("[DRAW DEBUG] Instance 0 Draw: matrixPos = %u, joints = %d\n",
        //      matrixPos, instance->getJointMatrixSize());
      }
      //Logger::log(1, "[DRAW DEBUG] Instance: %d, Offset: %d\n", instanceCounter, matrixPos);
      mGltfGPUShader.setUniformValue((int)matrixPos); // Передаем индекс стартовой матрицы
      instance->getModel()->draw();

      // ВАЖНО: это число должно быть равно размеру вектора, 
      // который вернул этот же instance в цикле выше!
      //matrixPos += instance->getJointMatrixSize();
      matrixPos += static_cast<unsigned int>(256); // Теперь здесь всегда 256!
      instanceCounter++; 
  }

  unsigned int dqPos = 0;
  
  mGltfGPUDualQuatShader.use();
  for (const auto &instance : mGltfDQInstances) {
    mGltfGPUDualQuatShader.setUniformValue(dqPos);
    instance->getModel()->draw();
    dqPos += instance->getJointDualQuatsSize();
  }

  // draw the coordinate arrow WITH depth buffer 
  if (mCoordArrowsLineIndexCount > 0) {
    mLineShader.use();
    mVertexBuffer.bindAndDraw(GL_LINES, mSkeletonLineIndexCount, mCoordArrowsLineIndexCount);
  }

  // draw the skeleton, disable depth test to overlay 
  if (mSkeletonLineIndexCount > 0) {
    glDisable(GL_DEPTH_TEST);
    mLineShader.use();
    mVertexBuffer.bindAndDraw(GL_LINES, 0, mSkeletonLineIndexCount);
    glEnable(GL_DEPTH_TEST);
  }
  
  mFramebuffer.unbind();

  /* blit color buffer to screen */
  mFramebuffer.drawToScreen();

  mUIGenerateTimer.start();

  ModelSettings settings = mGltfInstances.at(selectedInstance)->getInstanceSettings();
  mUserInterface.createFrame(mRenderData, settings, mShowMenu);
  mGltfInstances.at(selectedInstance)->setInstanceSettings(settings);
  mGltfInstances.at(selectedInstance)->checkForUpdates();

  mRenderData.rdUIGenerateTime = mUIGenerateTimer.stop();

  mUIDrawTimer.start();
  mUserInterface.render();
  mRenderData.rdUIDrawTime = mUIDrawTimer.stop();

  mLastTickTime = tickTime;
}
void OGLRenderer::updatePlayerCharacter(float deltaTime) {
    auto& enty = mGltfInstances.at(1);
    if (!enty->getRigidBody()) return;

    btRigidBody* body = enty->getRigidBody();
    body->activate(true);
    body->setAngularFactor(btVector3(0, 1, 0));

    // --- 0. ТАЙМЕРЫ СПЕЦ-ДЕЙСТВИЙ ---
    static float dashDurationTimer = 0.0f;
    static float dashCooldownTimer = 0.0f;

    // НОВОЕ: Таймеры жесткой фиксации для атак (Action Lock)
    static float attackDurationTimer = 0.0f; // Сколько еще будет длиться удар
    static int currentAttackAnim = EntyAnim::Idle; // Какой именно удар мы сейчас бьем
    static float attackTimeAccumulator = 0.0f; // Наше ручное время анимации с нуля

    if (dashDurationTimer > 0.0f) dashDurationTimer -= deltaTime;
    if (dashCooldownTimer > 0.0f) dashCooldownTimer -= deltaTime;

    if (attackDurationTimer > 0.0f) {
        attackDurationTimer -= deltaTime;
        attackTimeAccumulator += deltaTime; // Мотаем время анимации вперед!
    }


    // --- 1. ЧИТАЕМ КНОПКИ ДЛЯ АНИМАЦИЙ ---
    bool keyC = glfwGetKey(mRenderData.rdWindow, GLFW_KEY_C) == GLFW_PRESS;
    bool keyG = glfwGetKey(mRenderData.rdWindow, GLFW_KEY_G) == GLFW_PRESS;
    bool keyE = glfwGetKey(mRenderData.rdWindow, GLFW_KEY_E) == GLFW_PRESS;
    bool keyZ = glfwGetKey(mRenderData.rdWindow, GLFW_KEY_Z) == GLFW_PRESS;
    bool keyH = glfwGetKey(mRenderData.rdWindow, GLFW_KEY_H) == GLFW_PRESS;
    bool keySpace = glfwGetKey(mRenderData.rdWindow, GLFW_KEY_SPACE) == GLFW_PRESS;
    bool mouseLMB = glfwGetMouseButton(mRenderData.rdWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

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


    // --- 2. ПРАВИЛЬНАЯ ПРОВЕРКА ЗЕМЛИ ---
    bool isOnGround = false;
    btTransform currentTransform;
    body->getMotionState()->getWorldTransform(currentTransform);
    btVector3 rayStart = currentTransform.getOrigin();

    btVector3 rayEnd = rayStart + btVector3(0.0f, -1.3f, 0.0f);
    btCollisionWorld::ClosestRayResultCallback rayCallback(rayStart, rayEnd);
    mDynamicsWorld->rayTest(rayStart, rayEnd, rayCallback);

    if (rayCallback.hasHit() && rayCallback.m_collisionObject != body) {
        isOnGround = true;
    }


    // --- 3. ЛОГИКА ЗАПУСКА ДЭША И АТАК ---
    if (keyE && isOnGround && dashDurationTimer <= 0.0f && dashCooldownTimer <= 0.0f && attackDurationTimer <= 0.0f) {
        dashDurationTimer = 0.3f;
        dashCooldownTimer = 1.5f;
    }

    // НОВОЕ: Захватываем атаку, только если сейчас не бьем и не дэшим
    if (isOnGround && attackDurationTimer <= 0.0f && dashDurationTimer <= 0.0f) {
        if (keySpace && mouseLMB) {
            currentAttackAnim = EntyAnim::FlyKick;
            attackDurationTimer = enty->getAnimationEndTime(currentAttackAnim);
            attackTimeAccumulator = 0.0f; // Сбрасываем таймер в 0!
        }
        else if (mouseLMB) {
            currentAttackAnim = EntyAnim::Boxing;
            attackDurationTimer = enty->getAnimationEndTime(currentAttackAnim);
            attackTimeAccumulator = 0.0f;
        }
        else if (keyZ) {
            currentAttackAnim = EntyAnim::DoubleJumpKick;
            attackDurationTimer = enty->getAnimationEndTime(currentAttackAnim);
            attackTimeAccumulator = 0.0f;
        }
        else if (keyC) {
            currentAttackAnim = EntyAnim::KICK;
            attackDurationTimer = enty->getAnimationEndTime(currentAttackAnim);
            attackTimeAccumulator = 0.0f;
        }
    }


    // --- 4. ДЕРЕВО ПРИОРИТЕТОВ АНИМАЦИЙ ---
    int currentTargetAnim = EntyAnim::Idle;

    if (dashDurationTimer > 0.0f) {
        currentTargetAnim = EntyAnim::DashBack;
    }
    // НОВОЕ: Если таймер атаки тикает, мы ЖЕСТКО держим эту анимацию
    else if (attackDurationTimer > 0.0f) {
        currentTargetAnim = currentAttackAnim;
    }
    // Дальше идут обычные прерываемые анимации
    else if (!isOnGround) {
        currentTargetAnim = EntyAnim::Jump;
    }
    else if (keyG) {
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


    // --- 5. СПЕЦИАЛЬНЫЕ СОСТОЯНИЯ (БЛОКИРОВКА И ДЭШ) ---
    bool isMovementBlocked = (
        attackDurationTimer > 0.0f || // Блокируем ходьбу во время удара
        currentTargetAnim == EntyAnim::Dance1 ||
        currentTargetAnim == EntyAnim::DanceWin
        );

    bool isDashing = (currentTargetAnim == EntyAnim::DashBack);

    if (isMovementBlocked || isDashing) {
        moveDir = glm::vec3(0.0f);
    }


    // --- 6. ФИЗИКА ДВИЖЕНИЯ ---
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
    // НОВОЕ: Физический рывок вперед для конкретных атак
    else if (attackDurationTimer > 0.0f && (currentAttackAnim == EntyAnim::FlyKick || currentAttackAnim == EntyAnim::DoubleJumpKick)) {
        btTransform trans;
        body->getMotionState()->getWorldTransform(trans);

        // Вектор "Вперед" для нашей модели
        btVector3 forwardDir = quatRotate(trans.getRotation(), btVector3(0, 0, 1));

        float attackMoveSpeed = 0.0f; // По умолчанию стоим на месте (замах или выход из удара)

        // 1. Настройки для FLY KICK (удар в полете)
        if (currentAttackAnim == EntyAnim::FlyKick) {
            // Энти летит вперед только с 0.2 по 0.6 секунды анимации. 
            // (Замени эти цифры на те моменты, когда в Блендере она реально летит)
            if (attackTimeAccumulator > 0.0f && attackTimeAccumulator < 1.2f) {
                attackMoveSpeed = 20.0f; // Очень быстрый рывок вперед
            }
        }
        // 2. Настройки для DOUBLE JUMP KICK (двойной удар)
        else if (currentAttackAnim == EntyAnim::DoubleJumpKick) {
            // Допустим, она делает первый шаг на 0.1с, а второй рывок на 0.5с
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
    // Обычная ходьба и бег
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
    // Обычное торможение (включая торможение при обычных атаках типа Boxing)
    else {
        float decel = 15.0f * deltaTime;
        btVector3 targetVel(0.0f, currentVel.y(), 0.0f);
        btVector3 finalVel = currentVel.lerp(targetVel, decel);
        body->setLinearVelocity(finalVel);
        body->setAngularVelocity(btVector3(0, 0, 0));
    }


    // --- 7. ФИЗИЧЕСКИЙ АКТ ПРЫЖКА ---
    if (keySpace && !mWasSpacePressed && isOnGround && !isMovementBlocked && !isDashing) {
        float jumpForce = 340.0f;
        body->applyCentralImpulse(btVector3(0.0f, jumpForce, 0.0f));
        isOnGround = false;
    }
    mWasSpacePressed = keySpace;


    // --- 8. МГНОВЕННОЕ ПЕРЕКЛЮЧЕНИЕ АНИМАЦИИ ---
    if (currentTargetAnim != settings.msCrossBlendDestAnimClip) {
        settings.msCrossBlendDestAnimClip = currentTargetAnim;
        settings.msAnimClip = currentTargetAnim;

        settings.msAnimCrossBlendFactor = 1.0f;
        settings.msBlendingMode = blendMode::crossfade;

        settings.msAnimTimePosition = 0.0f;
        settingsChanged = true;
    }

    // --- 9. ОТПРАВКА ДАННЫХ В ДВИЖОК ---
    bool forceManualUpdate = false;

    // НОВОЕ: Перехватываем управление временем движка
    if (attackDurationTimer > 0.0f) {
        settings.msPlayAnimation = false; // Вырубаем глобальные часы
        settings.msAnimTimePosition = attackTimeAccumulator; // Кормим время вручную!
        forceManualUpdate = true; // Заставляем движок обновлять кости каждый кадр
    }
    else {
        settings.msPlayAnimation = true; // Для бега, дэша и айдла возвращаем автовоспроизведение
    }

    if (settingsChanged || forceManualUpdate) {
        if (currentTargetAnim == EntyAnim::DashBack) {
            settings.msAnimSpeed = 2.4f;
        }
        else {
            settings.msAnimSpeed = 1.0f;
        }

        enty->setInstanceSettings(settings);
        enty->checkForUpdates();
    }
}
void OGLRenderer::cleanup() {

    if (mDynamicsWorld) {
        // Удаляем все RigidBody из мира (опционально, но хороший тон)
        for (int i = mDynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
            btCollisionObject* obj = mDynamicsWorld->getCollisionObjectArray()[i];
            mDynamicsWorld->removeCollisionObject(obj);
            delete obj;
        }
        delete mDynamicsWorld;
        delete mSolver;
        delete mOverlappingPairCache;
        delete mDispatcher;
        delete mCollisionConfiguration;
    }

  for (int i = 0; i < mGltfModels.size(); ++i) {
    mGltfModels.at(i)->cleanup();
    mGltfModels.at(i).reset();
  }

  mGltfGPUDualQuatShader.cleanup();
  mGltfGPUShader.cleanup();
  mUserInterface.cleanup();
  mLineShader.cleanup();
  mVertexBuffer.cleanup();
  mGltfShaderStorageBuffer.cleanup();
  mGltfDualQuatSSBuffer.cleanup();
  mUniformBuffer.cleanup();
  mFramebuffer.cleanup();
}
