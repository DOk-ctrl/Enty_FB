#define GLM_ENABLE_EXPERIMENTAL
#include <algorithm>
#include <chrono>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/dual_quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "GltfModel.h"
#include "Logger.h"
#include "Tools.h"
bool GltfModel::loadModel(OGLRenderData& renderData,
    std::string modelFilename, std::string textureFilename,
    std::string normalFilename, std::string metalFilename, std::string roughFilename, std::string emitFilename,
    std::string packedFilename, std::string coatRoughFilename) {
    // Загрузка Base Color
    if (!mTex.loadTexture(textureFilename, false)) {
        Logger::log(1, "%s: texture loading failed\n", __FUNCTION__);
        return false;
    }

    // Загрузка Normal map(если передана)
    if (!normalFilename.empty()) {
        if (mNormalTex.loadTexture(normalFilename, false)) {
            mHasNormalMap = true;
            Logger::log(1, "%s: Normal map successfully loaded\n", __FUNCTION__, normalFilename.c_str());
        }
    }
    // Загрузка Metallic map
    if (!metalFilename.empty()) {
        if (mMetalTex.loadTexture(metalFilename, false)) {
            mHasMetalMap = true;
            Logger::log(1, "%s: Metallic map successfully loaded: %s\n", __FUNCTION__, metalFilename.c_str());
        }
    }

    // Загрузка Roughness map
    if (!roughFilename.empty()) {
        if (mRoughTex.loadTexture(roughFilename, false)) {
            mHasRoughMap = true;
            Logger::log(1, "%s: Roughness map successfully loaded: %s\n", __FUNCTION__, roughFilename.c_str());
        }
    }

    if (!emitFilename.empty()) {
        if (mEmitTex.loadTexture(emitFilename, false)) {
            mHasEmitMap = true;
            Logger::log(1, "%s: Emission map successfully loaded: %s\n", __FUNCTION__, emitFilename.c_str());
        }
    }
    // Загрузка Channel Packed Map
    if (!packedFilename.empty()) {
        if (mPackedTex.loadTexture(packedFilename, false)) {
            mHasPackedMap = true;
            Logger::log(1, "%s: Packed map successfully loaded: %s\n", __FUNCTION__, packedFilename.c_str());
        }
    }

    // Загрузка Coat Roughness Map
    if (!coatRoughFilename.empty()) {
        if (mCoatRoughTex.loadTexture(coatRoughFilename, false)) {
            mHasCoatRoughMap = true;
            Logger::log(1, "%s: Coat Roughness map successfully loaded: %s\n", __FUNCTION__, coatRoughFilename.c_str());
        }
    }
    Logger::log(1, "%s: glTF model texture '%s' successfully loaded\n", __FUNCTION__,
        modelFilename.c_str());

    mModel = std::make_shared<tinygltf::Model>();

    tinygltf::TinyGLTF gltfLoader;
    std::string loaderErrors;
    std::string loaderWarnings;
    bool result = false;

    result = gltfLoader.LoadASCIIFromFile(mModel.get(), &loaderErrors, &loaderWarnings, modelFilename);

    if (!loaderWarnings.empty()) {
        Logger::log(1, "%s: warnings while loading glTF model:\n%s\n", __FUNCTION__,
            loaderWarnings.c_str());
    }

    if (!loaderErrors.empty()) {
        Logger::log(1, "%s: errors while loading glTF model:\n%s\n", __FUNCTION__,
            loaderErrors.c_str());
    }

    if (!result) {
        Logger::log(1, "%s error: could not load file '%s'\n", __FUNCTION__,
            modelFilename.c_str());
        return false;
    }

    mModelFilename = modelFilename;

    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    /* extract position, normal, texture coords, and indices */
    createVertexBuffers();
    createIndexBuffer();

    //14.01.26
    uploadVertexBuffers();
    uploadIndexBuffer();

    glBindVertexArray(0);

    /* extract joints, weights, and invers bind matrices*/
    if (!mModel->skins.empty()) {
        getJointData();
        getWeightData();
        getInvBindMatrices();
    }
    else {
        Logger::log(1, "%s: model has no skins (static mesh)\n", __FUNCTION__);
    }
    mNodeCount = mModel->nodes.size();

    /* extract animation data */
    getAnimations();

    return true;
}


std::string GltfModel::getModelFilename() {
  return mModelFilename;
}

int GltfModel::getNodeCount() {
  return mNodeCount;
}

GltfNodeData GltfModel::getGltfNodes() {
  GltfNodeData nodeData{};
  // 1. Проверка: загружена ли модель вообще?
  if (!mModel) {
      Logger::log(1, "%s ERROR: mModel is nullptr! Model was not loaded correctly.\n", __FUNCTION__);
      return nodeData;
  }

  // 2. Проверка: есть ли сцены в модели?
  if (mModel->scenes.empty()) {
      Logger::log(1, "%s ERROR: Model has no scenes!\n", __FUNCTION__);
      return nodeData;
  }

  const auto& defaultScene = mModel->scenes.at(0);

  // 3. Проверка: есть ли ноды в первой сцене?
  if (defaultScene.nodes.empty()) {
      Logger::log(1, "%s ERROR: Scene 0 has no root nodes!\n", __FUNCTION__);
      return nodeData;
  }
  int rootNodeNum = mModel->scenes.at(0).nodes.at(0);
  Logger::log(2, "%s: model has %i nodes, root node is %i\n", __FUNCTION__,
    mNodeCount, rootNodeNum);

  nodeData.rootNode = GltfNode::createRoot(rootNodeNum);

  getNodeData(nodeData.rootNode);
  getNodes(nodeData.rootNode);

  nodeData.nodeList.resize(mNodeCount);
  nodeData.nodeList.at(rootNodeNum) = nodeData.rootNode;
  getNodeList(nodeData.nodeList, rootNodeNum);

  return nodeData;
}

void GltfModel::getJointData() {
  std::string jointsAccessorAttrib = "JOINTS_0";
  int jointsAccessor = mModel->meshes.at(0).primitives.at(0).attributes.at(jointsAccessorAttrib);
  Logger::log(1, "%s: using accessor %i to get %s\n", __FUNCTION__, jointsAccessor,
    jointsAccessorAttrib.c_str());

  const tinygltf::Accessor &accessor = mModel->accessors.at(jointsAccessor);
  const tinygltf::BufferView &bufferView = mModel->bufferViews.at(accessor.bufferView);
  const tinygltf::Buffer &buffer = mModel->buffers.at(bufferView.buffer);

  int jointVecSize = accessor.count;
  Logger::log(1, "%s: %i short vec4 in JOINTS_0\n", __FUNCTION__, jointVecSize);
  mJointVec.resize(jointVecSize);

  std::memcpy(mJointVec.data(), &buffer.data.at(0) + bufferView.byteOffset,
    bufferView.byteLength);

  mNodeToJoint.resize(mModel->nodes.size());

  const tinygltf::Skin &skin = mModel->skins.at(0);
  for (int i = 0; i < skin.joints.size(); ++i) {
    int destinationNode = skin.joints.at(i);
    mNodeToJoint.at(destinationNode) = i;
    Logger::log(2, "%s: joint %i affects node %i\n", __FUNCTION__, i, destinationNode);
  }
  Logger::log(1, "--- [MODEL JOINT CHECK] ---\n");
  Logger::log(1, "Skin joints size: %zu\n", skin.joints.size());
  Logger::log(1, "NodeToJoint vector size: %zu\n", mNodeToJoint.size());
}

void GltfModel::getWeightData() {
  std::string weightsAccessorAttrib = "WEIGHTS_0";
  int weightAccessor = mModel->meshes.at(0).primitives.at(0).attributes.at(weightsAccessorAttrib);
  Logger::log(1, "%s: using accessor %i to get %s\n", __FUNCTION__, weightAccessor,
    weightsAccessorAttrib.c_str());

  const tinygltf::Accessor &accessor = mModel->accessors.at(weightAccessor);
  const tinygltf::BufferView &bufferView = mModel->bufferViews.at(accessor.bufferView);
  const tinygltf::Buffer &buffer = mModel->buffers.at(bufferView.buffer);

  int weightVecSize = accessor.count;
  Logger::log(1, "%s: %i vec4 in WEIGHTS_0\n", __FUNCTION__, weightVecSize);
  mWeightVec.resize(weightVecSize);

  std::memcpy(mWeightVec.data(), &buffer.data.at(0) + bufferView.byteOffset,
    bufferView.byteLength);
}

void GltfModel::getInvBindMatrices() {
  const tinygltf::Skin &skin = mModel->skins.at(0);
  int invBindMatAccessor = skin.inverseBindMatrices;

  const tinygltf::Accessor &accessor = mModel->accessors.at(invBindMatAccessor);
  const tinygltf::BufferView &bufferView = mModel->bufferViews.at(accessor.bufferView);
  const tinygltf::Buffer &buffer = mModel->buffers.at(bufferView.buffer);

  mInverseBindMatrices.resize(skin.joints.size());

  std::memcpy(mInverseBindMatrices.data(), &buffer.data.at(0) + bufferView.byteOffset,
    bufferView.byteLength);
}

void GltfModel::getAnimations() {
    // 1. Выделяем память под массив клипов
    mAnimClips.reserve(mModel->animations.size());

    for (const auto& anim : mModel->animations) {
        // Оставляем ТОЛЬКО один лог на целую анимацию (ни в коем случае не внутри цикла по каналам!)
        Logger::log(1, "%s: loading animation '%s' with %i channels\n",
            __FUNCTION__, anim.name.c_str(), static_cast<int>(anim.channels.size()));

        auto clip = std::make_shared<GltfAnimationClip>(anim.name);

        // 2. Резервируем память под 504 канала РАЗОМ
        clip->reserveChannels(anim.channels.size());

        for (const auto& channel : anim.channels) {
            clip->addChannel(mModel, anim, channel);
        }

        mAnimClips.push_back(std::move(clip)); // std::move чтобы не инкрементировать счетчик shared_ptr лишний раз
    }
}

std::vector<std::shared_ptr<GltfAnimationClip>> GltfModel::getAnimClips() {
  return mAnimClips;
}

void GltfModel::getNodes(std::shared_ptr<GltfNode> treeNode) {
  int nodeNum = treeNode->getNodeNum();
  std::vector<int> childNodes = mModel->nodes.at(nodeNum).children;

  /* remove the child node with skin/mesh metadata, confuses skeleton */
  auto removeIt = std::remove_if(childNodes.begin(), childNodes.end(),
    [&](int num) { return mModel->nodes.at(num).skin != -1; }
  );
  childNodes.erase(removeIt, childNodes.end());

  treeNode->addChilds(childNodes);

  for (auto &childNode : treeNode->getChilds()) {
    getNodeData(childNode);
    getNodes(childNode);
  }
}

std::vector<std::shared_ptr<GltfNode>> GltfModel::getNodeList(
    std::vector<std::shared_ptr<GltfNode>> &nodeList, int nodeNum) {
  for (auto &childNode : nodeList.at(nodeNum)->getChilds()) {
    int childNodeNum = childNode->getNodeNum();
    nodeList.at(childNodeNum) = childNode;
    getNodeList(nodeList, childNodeNum);
  }

  return nodeList;
}

std::vector<glm::mat4> GltfModel::getInverseBindMatrices() {
  return mInverseBindMatrices;
}

std::vector<int> GltfModel::getNodeToJoint() {
  return mNodeToJoint;
}

void GltfModel::getNodeData(std::shared_ptr<GltfNode> treeNode) {
  int nodeNum = treeNode->getNodeNum();
  const tinygltf::Node &node = mModel->nodes.at(nodeNum);
  treeNode->setNodeName(node.name);

  if (node.translation.size()) {
    treeNode->setTranslation(glm::make_vec3(node.translation.data()));
  } else {
    treeNode->setTranslation(glm::vec3(0.0f));
  }

  if (node.rotation.size()) {
    treeNode->setRotation(glm::make_quat(node.rotation.data()));
  } else {
    treeNode->setRotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
  }

  if (node.scale.size()) {
    treeNode->setScale(glm::make_vec3(node.scale.data()));
  } else {
    treeNode->setScale(glm::vec3(1.0f));
  }

  treeNode->calculateNodeMatrix();
}

void GltfModel::resetNodeData(std::shared_ptr<GltfNode> treeNode) {
  getNodeData(treeNode);

  for (auto &childNode : treeNode->getChilds()) {
    resetNodeData(childNode);
  }
}
// Вспомогательная функция для рекурсивного обхода узлов
void GltfModel::processNodePhysics(const tinygltf::Model* model, int nodeIdx, const glm::mat4& parentMatrix, btTriangleMesh* meshInterface) {
    const auto& node = model->nodes[nodeIdx];

    // 1. ВЫЧИСЛЯЕМ ЛОКАЛЬНУЮ МАТРИЦУ УЗЛА (с приведением типов)
    glm::mat4 localMatrix(1.0f);

    if (node.matrix.size() == 16) {
        // Переносим double в массив float
        float matData[16];
        for (int i = 0; i < 16; ++i) matData[i] = static_cast<float>(node.matrix[i]);
        localMatrix = glm::make_mat4(matData);
    }
    else {
        // Обработка Translation
        if (node.translation.size() == 3) {
            localMatrix = glm::translate(localMatrix, glm::vec3(
                static_cast<float>(node.translation[0]),
                static_cast<float>(node.translation[1]),
                static_cast<float>(node.translation[2])
            ));
        }
        // Обработка Rotation (Quaternion)
        if (node.rotation.size() == 4) {
            glm::quat q(
                static_cast<float>(node.rotation[3]), // w
                static_cast<float>(node.rotation[0]), // x
                static_cast<float>(node.rotation[1]), // y
                static_cast<float>(node.rotation[2])  // z
            );
            localMatrix *= glm::mat4_cast(q);
        }
        // Обработка Scale
        if (node.scale.size() == 3) {
            localMatrix = glm::scale(localMatrix, glm::vec3(
                static_cast<float>(node.scale[0]),
                static_cast<float>(node.scale[1]),
                static_cast<float>(node.scale[2])
            ));
        }
    }

    // Комбинируем с родительской матрицей
    glm::mat4 nodeMatrix = parentMatrix * localMatrix;

    // 2. ОБРАБОТКА МЕША (если он есть в узле)
    if (node.mesh >= 0) {
        const auto& gltfMesh = model->meshes[node.mesh];
        for (const auto& primitive : gltfMesh.primitives) {
            if (primitive.attributes.find("POSITION") == primitive.attributes.end()) continue;
            if (primitive.indices < 0) continue;

            // Доступ к позициям
            const tinygltf::Accessor& posAccessor = model->accessors[primitive.attributes.at("POSITION")];
            const tinygltf::BufferView& posView = model->bufferViews[posAccessor.bufferView];
            const tinygltf::Buffer& posBuffer = model->buffers[posView.buffer];
            const uint8_t* posData = &posBuffer.data[posView.byteOffset + posAccessor.byteOffset];
            int posByteStride = posAccessor.ByteStride(posView);
            if (posByteStride == 0) posByteStride = sizeof(float) * 3;

            // Доступ к индексам
            const tinygltf::Accessor& indexAccessor = model->accessors[primitive.indices];
            const tinygltf::BufferView& indexView = model->bufferViews[indexAccessor.bufferView];
            const tinygltf::Buffer& indexBuffer = model->buffers[indexView.buffer];
            const uint8_t* indexData = &indexBuffer.data[indexView.byteOffset + indexAccessor.byteOffset];
            int indexCount = indexAccessor.count;

            // Цикл по треугольникам
            for (int i = 0; i < indexCount; i += 3) {
                int i0 = 0, i1 = 0, i2 = 0;

                if (indexAccessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT) {
                    const uint16_t* indices = reinterpret_cast<const uint16_t*>(indexData);
                    i0 = indices[i]; i1 = indices[i + 1]; i2 = indices[i + 2];
                }
                else if (indexAccessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT) {
                    const uint32_t* indices = reinterpret_cast<const uint32_t*>(indexData);
                    i0 = indices[i]; i1 = indices[i + 1]; i2 = indices[i + 2];
                }
                else if (indexAccessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE) {
                    const uint8_t* indices = indexData;
                    i0 = indices[i]; i1 = indices[i + 1]; i2 = indices[i + 2];
                }

                const float* p0 = reinterpret_cast<const float*>(posData + i0 * posByteStride);
                const float* p1 = reinterpret_cast<const float*>(posData + i1 * posByteStride);
                const float* p2 = reinterpret_cast<const float*>(posData + i2 * posByteStride);

                // Трансформируем вершины в мировые координаты меша
                glm::vec4 v0 = nodeMatrix * glm::vec4(p0[0], p0[1], p0[2], 1.0f);
                glm::vec4 v1 = nodeMatrix * glm::vec4(p1[0], p1[1], p1[2], 1.0f);
                glm::vec4 v2 = nodeMatrix * glm::vec4(p2[0], p2[1], p2[2], 1.0f);

                meshInterface->addTriangle(
                    btVector3(v0.x, v0.y, v0.z),
                    btVector3(v1.x, v1.y, v1.z),
                    btVector3(v2.x, v2.y, v2.z)
                );
            }
        }
    }

    // 3. РЕКУРСИЯ
    for (int childIdx : node.children) {
        processNodePhysics(model, childIdx, nodeMatrix, meshInterface);
    }
}

btBvhTriangleMeshShape* GltfModel::getPhysicsShape() {
    btTriangleMesh* mesh = new btTriangleMesh();

    // Находим корневые узлы сцены (обычно scene 0)
    const tinygltf::Scene& scene = mModel->scenes[mModel->defaultScene > -1 ? mModel->defaultScene : 0];

    for (int nodeIdx : scene.nodes) {
        processNodePhysics(mModel.get(), nodeIdx, glm::mat4(1.0f), mesh);
    }

    return new btBvhTriangleMeshShape(mesh, true);
}

void GltfModel::createVertexBuffers() {
    // 1. Создаем и привязываем VAO
    if (mVAO == 0) glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    // 2. Создаем один главный VBO для всех вершинных данных модели
    // Мы будем использовать mVertexVBO[0] как хранилище всего .bin файла
    if (mVertexVBO.empty()) mVertexVBO.resize(16, 0);
    if (mVertexVBO[0] == 0) glGenBuffers(1, &mVertexVBO[0]);

    // Привязываем его сразу
    glBindBuffer(GL_ARRAY_BUFFER, mVertexVBO[0]);

    // 3. Берем ПЕРВЫЙ примитив ПЕРВОГО меша просто чтобы инициализировать слоты VAO
    // Это "шаблон" для видеокарты, чтобы она знала, какие атрибуты вообще существуют.
    // Реальные смещения мы будем переназначать в draw() для каждого куска 2B.
    if (mModel->meshes.empty() || mModel->meshes.at(0).primitives.empty()) {
        return;
    }

    const auto& attributesMap = mModel->meshes.at(0).primitives.at(0).attributes;

    for (const auto& [attribName, accessorIdx] : attributesMap) {
        // Проверяем, нужен ли нам этот атрибут (твой стандартный фильтр)
        if (attributes.find(attribName) == attributes.end()) {
            continue;
        }

        const tinygltf::Accessor& accessor = mModel->accessors.at(accessorIdx);
        const tinygltf::BufferView& bufferView = mModel->bufferViews.at(accessor.bufferView);

        int loc = attributes.at(attribName);
        int dataSize = tinygltf::GetNumComponentsInType(accessor.type);
        int dataType = accessor.componentType;
        int stride = accessor.ByteStride(bufferView);

        // Смещение: Начало BufferView + Смещение внутри Accessor
        size_t totalOffset = bufferView.byteOffset + accessor.byteOffset;

        // Включаем слот и настраиваем указатель (пока для первой части тела)
        glEnableVertexAttribArray(loc);

        if (attribName == "JOINTS_0") {
            glVertexAttribIPointer(loc, dataSize, dataType, stride, (void*)totalOffset);
        }
        else {
            GLboolean norm = (attribName == "WEIGHTS_0" && dataType != GL_FLOAT) ? GL_TRUE : GL_FALSE;
            glVertexAttribPointer(loc, dataSize, dataType, norm, stride, (void*)totalOffset);
        }

        Logger::log(1, "%s: Attribute %s (loc %i) initialized at offset %zu\n",
            __FUNCTION__, attribName.c_str(), loc, totalOffset);
    }

    // Отвязываем всё для безопасности
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GltfModel::createIndexBuffer() {

    glBindVertexArray(mVAO);

  glGenBuffers(1, &mIndexVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVBO);

  /* do NOT unbind the element buffer here */
  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void GltfModel::uploadVertexBuffers() {
    // 15
    glBindVertexArray(mVAO);

    // Убедимся, что VBO создан 17.01
    if (mVertexVBO.empty()) mVertexVBO.resize(16);
    if (mVertexVBO[0] == 0) glGenBuffers(1, &mVertexVBO[0]);

    const tinygltf::Buffer& allData = mModel->buffers.at(0);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, allData.data.size(),
        allData.data.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // поменял фор 08.01.26 с 5 до сайз()
  /* or (int i = 0; i < mVertexVBO.size(); ++i) {
      if (mVertexVBO.at(i) == 0 || i >= mAttribAccessors.size() || mAttribAccessors.at(i) == -1) {
          continue;
      }
    const tinygltf::Accessor& accessor = mModel->accessors.at(mAttribAccessors.at(i));
    const tinygltf::BufferView& bufferView = mModel->bufferViews.at(accessor.bufferView);
    const tinygltf::Buffer& buffer = mModel->buffers.at(bufferView.buffer);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexVBO.at(i));
    glBufferData(GL_ARRAY_BUFFER, bufferView.byteLength,
     &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }*/
  //15
  glBindVertexArray(0);
}

void GltfModel::uploadIndexBuffer() {
    // 15
    if (mIndexVBO == 0) return; // Нет буфера — нет загрузки
    glBindVertexArray(mVAO);
  /* buffer for vertex indices 
  const tinygltf::Primitive& primitives = mModel->meshes.at(0).primitives.at(0);
  const tinygltf::Accessor& indexAccessor = mModel->accessors.at(primitives.indices);
  const tinygltf::BufferView& indexBufferView = mModel->bufferViews.at(indexAccessor.bufferView);
  const tinygltf::Buffer& indexBuffer = mModel->buffers.at(indexBufferView.buffer);
  */
      const tinygltf::Buffer& allData = mModel->buffers.at(0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, allData.data.size(), allData.data.data(), GL_STATIC_DRAW);

      // 15
      glBindVertexArray(0);
}

int GltfModel::getTriangleCount() {
    if (mModel->meshes.empty() || mModel->meshes.at(0).primitives.empty()) {
        return 0;
    }

    const tinygltf::Primitive& primitives = mModel->meshes.at(0).primitives.at(0);

    // ГЕПА-ФИКС: Если индексов нет (-1), возвращаем 0 или считаем вершины (если нужно)
    if (primitives.indices < 0) {
        return 0;
    }

    const tinygltf::Accessor& indexAccessor = mModel->accessors.at(primitives.indices);

    unsigned int triangles = 0;
    switch (primitives.mode) {
    case TINYGLTF_MODE_TRIANGLES:
        // Здесь тоже на всякий случай static_cast
        triangles = static_cast<unsigned int>(indexAccessor.count) / 3;
        break;
    default:
        // Для других режимов пока вернем 0, чтобы не падать
        break;
    }
    return static_cast<int>(triangles);
}
void GltfModel::setTextureQuality(int level)
{
    mTex.setMipmapLevel(level);

    // Обновляем все остальные PBR карты, если они загружены
    if (mHasNormalMap) mNormalTex.setMipmapLevel(level);
    if (mHasMetalMap) mMetalTex.setMipmapLevel(level);
    if (mHasRoughMap) mRoughTex.setMipmapLevel(level);
    if (mHasEmitMap) mEmitTex.setMipmapLevel(level);
    if (mHasPackedMap) mPackedTex.setMipmapLevel(level);
    if (mHasCoatRoughMap) mCoatRoughTex.setMipmapLevel(level);
}
void GltfModel::setAnisotropicFiltering(float amount)
{
    mTex.setAnisotropy(amount);
    if (mHasNormalMap)    mNormalTex.setAnisotropy(amount);
    if (mHasMetalMap)     mMetalTex.setAnisotropy(amount);
    if (mHasRoughMap)     mRoughTex.setAnisotropy(amount);
    if (mHasEmitMap)      mEmitTex.setAnisotropy(amount);
    if (mHasPackedMap)    mPackedTex.setAnisotropy(amount);
    if (mHasCoatRoughMap) mCoatRoughTex.setAnisotropy(amount);
}
// 17 числа победная загрузка с костями
void GltfModel::draw() {
    // слот 0: Base Color
    glActiveTexture(GL_TEXTURE0);
    mTex.bind();

    // слот 1: Normal Map
    if (mHasNormalMap) {
        glActiveTexture(GL_TEXTURE1);
        mNormalTex.bind();
    }

    // слот 2: Metallic Map
    if (mHasPackedMap) {
        glActiveTexture(GL_TEXTURE2);
        mPackedTex.bind();
    }
    else if (mHasMetalMap) {
        glActiveTexture(GL_TEXTURE2);
        mMetalTex.bind();
    }

    // слот 3: Roughness Map
    if (mHasCoatRoughMap) {
        glActiveTexture(GL_TEXTURE3);
        mCoatRoughTex.bind();
    }
    else if (mHasRoughMap) {
        glActiveTexture(GL_TEXTURE3);
        mRoughTex.bind();
    }

    if (mHasEmitMap) {
        glActiveTexture(GL_TEXTURE4);
        mEmitTex.bind();
    }
    glBindVertexArray(mVAO);
    // Привязываем наш "Гига-VBO" (где лежит весь файл)
    glBindBuffer(GL_ARRAY_BUFFER, mVertexVBO[0]);
    // Привязываем индексы (где тоже лежит весь файл)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVBO);

    // Проходим по всем кускам 2B (голова, тело, руки...)
    for (const auto& mesh : mModel->meshes) {
        for (const auto& primitive : mesh.primitives) {

            // 1. НАСТРОЙКА УКАЗАТЕЛЕЙ ДЛЯ ЭТОГО КУСКА
            for (const auto& [name, accessorIdx] : primitive.attributes) {
                // Пропускаем неизвестные атрибуты
                if (attributes.find(name) == attributes.end()) continue;

                const tinygltf::Accessor& accessor = mModel->accessors.at(accessorIdx);
                const tinygltf::BufferView& view = mModel->bufferViews.at(accessor.bufferView);

                // ВАЖНО: Смещение = смещение View + смещение Accessor
                size_t totalOffset = view.byteOffset + accessor.byteOffset;
                int stride = accessor.ByteStride(view);
                int loc = attributes.at(name);

                int size = tinygltf::GetNumComponentsInType(accessor.type);
                int type = accessor.componentType;

                // Перенастраиваем указатель именно для этой части тела
                if (name == "JOINTS_0") {
                    glVertexAttribIPointer(loc, size, type, stride, (void*)totalOffset);
                }
                else {
                    bool norm = (name == "WEIGHTS_0" && type != GL_FLOAT);
                    glVertexAttribPointer(loc, size, type, norm, stride, (void*)totalOffset);
                }
                glEnableVertexAttribArray(loc);
            }

            // 2. ОТРИСОВКА
            if (primitive.indices >= 0) {
                const tinygltf::Accessor& indexAccessor = mModel->accessors.at(primitive.indices);
                const tinygltf::BufferView& indexView = mModel->bufferViews.at(indexAccessor.bufferView);

                // Смещение индексов тоже уникально для каждого куска
                size_t indexOffset = indexView.byteOffset + indexAccessor.byteOffset;

                glDrawElements(
                    (primitive.mode != -1) ? primitive.mode : GL_TRIANGLES,
                    indexAccessor.count,
                    indexAccessor.componentType,
                    (void*)indexOffset
                );
            }
        }
    }

    glBindVertexArray(0);

    // Отвязка текстур (в обратном порядке для красоты)
    if (mHasEmitMap) {
        glActiveTexture(GL_TEXTURE4);
        mEmitTex.unbind();
    }
    if (mHasCoatRoughMap) {
        glActiveTexture(GL_TEXTURE3);
        mCoatRoughTex.unbind();
    }
    else if (mHasRoughMap) {
        glActiveTexture(GL_TEXTURE3);
        mRoughTex.unbind();
    }
    if (mHasPackedMap) {
        glActiveTexture(GL_TEXTURE2);
        mPackedTex.unbind();
    }
    else if (mHasMetalMap) {
        glActiveTexture(GL_TEXTURE2);
        mMetalTex.unbind();
    }
    if (mHasNormalMap) {
        glActiveTexture(GL_TEXTURE1);
        mNormalTex.unbind();
    }
    glActiveTexture(GL_TEXTURE0);
    mTex.unbind();

}

void GltfModel::cleanup() {
  glDeleteBuffers(mVertexVBO.size(), mVertexVBO.data());
  glDeleteBuffers(1, &mVAO);
  glDeleteBuffers(1, &mIndexVBO);
  mTex.cleanup();
  mModel.reset();
}
