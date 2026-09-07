/* glTF model, ready to draw */
#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <glad/glad.h>
#include <tiny_gltf.h>

#include "Texture.h"
#include "GltfNode.h"
#include "GltfAnimationClip.h"

#include "OGLRenderData.h"
#include "btBulletDynamicsCommon.h"

struct GltfNodeData {
    std::shared_ptr<GltfNode> rootNode;
    std::vector<std::shared_ptr<GltfNode>> nodeList;
};

class GltfModel {
  public:
      // 🟧 Можно звать из std::async — не трогает OpenGL: грузит все 7 текстур
        // (пиксели в RAM) + парсит tinygltf + вытаскивает joints/weights/anim.
      bool loadModelData(std::string modelFilename, std::string textureFilename,
          std::string normalFilename = "", std::string metalFilename = "",
          std::string roughFilename = "", std::string emitFilename = "",
          std::string packedFilename = "", std::string coatRoughFilename = "");

      // 🛑 Только главный поток — заливает то, что собрал loadModelData(), в GPU
      bool uploadToGPU();

      bool loadModel(OGLRenderData& renderData, std::string modelFilename,
          std::string textureFilename, std::string normalFilename = "",
          std::string metalFilename = "", std::string roughFilename = "", std::string emitFilename = "",
          std::string packedFilename = "", std::string coatRoughFilename = "");

    void setTextureQuality(int level);
    void setAnisotropicFiltering(float amount);
    void draw();
    void cleanup();

    std::string getModelFilename();
    int getNodeCount();
    GltfNodeData getGltfNodes();
    int getTriangleCount();

    void uploadVertexBuffers();
    void uploadIndexBuffer();

    std::vector<glm::mat4> getInverseBindMatrices();
    std::vector<int> getNodeToJoint();

    std::vector<std::shared_ptr<GltfAnimationClip>> getAnimClips();

    void resetNodeData(std::shared_ptr<GltfNode> treeNode);

    Texture mTex;          // Base Color
    Texture mNormalTex;    // Normal Map
    Texture mMetalTex;     // Metallic Map
    Texture mRoughTex;     // Roughness Map
    Texture mEmitTex;
    Texture mPackedTex;      // Channel Packed Map
    Texture mCoatRoughTex;   // Coat Roughness Map
    // временное хранилище пиксельных данных между loadModelData() и uploadToGPU()
    TextureLoadResult mTexData, mNormalTexData, mMetalTexData, mRoughTexData,
        mEmitTexData, mPackedTexData, mCoatRoughTexData;
    bool mHasNormalMap = false;
    bool mHasMetalMap = false;
    bool mHasRoughMap = false;
    bool mHasEmitMap = false;
    bool mHasPackedMap = false;
    bool mHasCoatRoughMap = false;

    void processNodePhysics(const tinygltf::Model* model, int nodeIdx, const glm::mat4& parentMatrix, btTriangleMesh* meshInterface);
    btBvhTriangleMeshShape* getPhysicsShape();

  private:
    void createVertexBuffers();
    void createIndexBuffer();

    void getJointData();
    void getWeightData();
    void getInvBindMatrices();
    void getAnimations();
    void getNodes(std::shared_ptr<GltfNode> treeNode);
    void getNodeData(std::shared_ptr<GltfNode> treeNode);
    std::vector<std::shared_ptr<GltfNode>> getNodeList(std::vector<std::shared_ptr<GltfNode>>
      &nodeList, int nodeNum);

    std::string mModelFilename;
    int mNodeCount = 0;

    std::shared_ptr<tinygltf::Model> mModel = nullptr;

    std::vector<glm::tvec4<uint16_t>> mJointVec{};
    std::vector<glm::vec4> mWeightVec{};
    std::vector<glm::mat4> mInverseBindMatrices{};

    std::vector<int> mAttribAccessors{};
    std::vector<int> mNodeToJoint{};

    std::vector<std::shared_ptr<GltfAnimationClip>> mAnimClips{};

    GLuint mVAO = 0;
    std::vector<GLuint> mVertexVBO{};
    GLuint mIndexVBO = 0;
    // 17.01
    size_t indexCount = 0;
    GLenum indexComponentType = 0;
    size_t indexByteOffset = 0;

    std::map<std::string, GLint> attributes =
    { {"POSITION", 0}, {"NORMAL", 1}, {"TEXCOORD_0", 2}, {"JOINTS_0", 3}, {"WEIGHTS_0", 4}, {"TANGENT", 5} };
    
};
