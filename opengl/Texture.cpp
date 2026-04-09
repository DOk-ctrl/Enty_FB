#define GLM_ENABLE_EXPERIMENTAL
#include <stb_image.h>

#include "Texture.h"
#include "Logger.h"

void Texture::cleanup() {
  glDeleteTextures(1, &mTexture);
}

bool Texture::loadTexture(std::string textureFilename, bool flipImage) {
  mTextureName = textureFilename;

  stbi_set_flip_vertically_on_load(flipImage);
  unsigned char *textureData = stbi_load(textureFilename.c_str(), &mTexWidth, &mTexHeight, &mNumberOfChannels, 0);

  if (!textureData) {
    Logger::log(1, "%s error: could not load file '%s'\n", __FUNCTION__, mTextureName.c_str());
    stbi_image_free(textureData);
    return false;
  }

  glGenTextures(1, &mTexture);
  glBindTexture(GL_TEXTURE_2D, mTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glGenTextures(1, &mTexture);
  glBindTexture(GL_TEXTURE_2D, mTexture);

  // ... (твои параметры фильтрации) ...

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  printf("[TEXTURE DEBUG] Attempting to upload: %s (%dx%d)\n", mTextureName.c_str(), mTexWidth, mTexHeight);

  if (mNumberOfChannels == 3) {
      // Используем внутренний формат RGBA8 для 24-битных картинок
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mTexWidth, mTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
  }
  else {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mTexWidth, mTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
  }

  GLenum uploadErr = glGetError();
  if (uploadErr != GL_NO_ERROR) {
      printf("!!! [TEXTURE ERROR] glTexImage2D failed: 0x%X\n", uploadErr);
  }
  else {
      printf("  glTexImage2D: OK\n");
  }

  printf("  Generating Mipmaps...\n");
  glGenerateMipmap(GL_TEXTURE_2D);

  GLenum mipErr = glGetError();
  if (mipErr != GL_NO_ERROR) {
      printf("!!! [TEXTURE ERROR] glGenerateMipmap failed: 0x%X\n", mipErr);
  }
  else {
      printf("  glGenerateMipmap: OK\n");
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  /*
  // выбор по количеству каналов какой формат загрузки использовать
  GLint maxTextureSize;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
  GLenum error = glGetError();
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  /*if (mNumberOfChannels == 3) {
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
     // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mTexWidth, mTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
      printf("Number of channels texture = 3 \n");
  }
  else {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mTexWidth, mTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
      printf("Number of channels texture = 4 \n");

  }
  
  if (error != GL_NO_ERROR) {
      printf("OpenGL Error: %x\n", error);
  }
  glGenerateMipmap(GL_TEXTURE_2D);
  
  glBindTexture(GL_TEXTURE_2D, 0);
  */
  stbi_image_free(textureData);

  Logger::log(1, "%s: texture '%s' loaded (%dx%d, %d channels)\n", __FUNCTION__, mTextureName.c_str(), mTexWidth, mTexHeight, mNumberOfChannels);
  return true;
}

void Texture::bind() {
  glBindTexture(GL_TEXTURE_2D, mTexture);
}

void Texture::unbind() {
  glBindTexture(GL_TEXTURE_2D, 0);
}
