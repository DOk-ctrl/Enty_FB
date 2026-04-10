#define GLM_ENABLE_EXPERIMENTAL
#include "ShaderStorageBuffer.h"
#include "Logger.h"

void ShaderStorageBuffer::init(size_t bufferSize) {
  mBufferSize = bufferSize;

  glGenBuffers(1, &mShaderStorageBuffer);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, mShaderStorageBuffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, mBufferSize, NULL, GL_DYNAMIC_COPY);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ShaderStorageBuffer::uploadSsboData(std::vector<glm::mat4> bufferData, int bindingPoint) {
  if (bufferData.size() == 0) {
    //Logger::log(1, "[SSBO ERROR] bufferData is empty for binding %d\n", bindingPoint);
    return;
  }

  while (glGetError() != GL_NO_ERROR);

  size_t dataSize = bufferData.size() * sizeof(glm::mat4);

  // ËÎÃ ÄÀÍÍÛÕ
  //Logger::log(1, "[SSBO UPLOAD] Binding: %d, Elements: %zu, Total Size: %zu bytes\n", bindingPoint, bufferData.size(), dataSize);

  size_t bufferSize = bufferData.size() * sizeof(glm::mat4);

  size_t dataSize_1 = bufferData.size() * sizeof(glm::mat4);

  // ËÎÃ ÄÀÍÍÛÕ
  //Logger::log(1, "[SSBO UPLOAD] Binding: %d, Elements: %zu, Total Size: %zu bytes\n",bindingPoint, bufferData.size(), dataSize_1);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, mShaderStorageBuffer);

  if (dataSize_1 > mBufferSize) {
      //Logger::log(1, "[SSBO FATAL] Data size %zu exceeds allocated buffer size %zu!\n",   dataSize_1, mBufferSize);
  }

  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, bufferSize, bufferData.data());
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, bindingPoint, mShaderStorageBuffer, 0,
    bufferSize);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
      Logger::log(1, "[SSBO ERROR] glBufferSubData failed with error code: 0x%x\n", err);
  }
}

void ShaderStorageBuffer::uploadSsboData(std::vector<glm::mat2x4> bufferData, int bindingPoint) {
  if (bufferData.size() == 0) {
    return;
  }

  while (glGetError() != GL_NO_ERROR); // ×èñòèì îøèáêè

  size_t bufferSize = bufferData.size() * sizeof(glm::mat2x4);

  size_t dataSize = bufferData.size() * sizeof(glm::mat2x4);
 // Logger::log(1, "[SSBO DQ UPLOAD] Binding: %d, Elements: %zu, Size: %zu bytes\n", bindingPoint, bufferData.size(), dataSize);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, mShaderStorageBuffer);

  if (dataSize > mBufferSize) {
      Logger::log(1, "[SSBO FATAL DQ] Data size %zu exceeds buffer %zu!\n", dataSize, mBufferSize);
  }

  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, bufferSize, bufferData.data());
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, bindingPoint, mShaderStorageBuffer, 0,
    bufferSize);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
      Logger::log(1, "[SSBO DQ ERROR] Failed with code: 0x%x\n", err);
  }
}

void ShaderStorageBuffer::cleanup() {
  glDeleteBuffers(1, &mShaderStorageBuffer);
}
