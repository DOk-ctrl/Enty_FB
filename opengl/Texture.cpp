#define GLM_ENABLE_EXPERIMENTAL
#include <stb_image.h>
#include<vector>
#include "Tools.h"
#include "Texture.h"
#include "Logger.h"

void Texture::cleanup() {
  glDeleteTextures(1, &mTexture);
}

bool Texture::loadTexture(std::string textureFilename, bool flipImage) {
    mTextureName = textureFilename;

    stbi_set_flip_vertically_on_load(flipImage);
    // загрузка без шифрования
    unsigned char* textureData = stbi_load(textureFilename.c_str(), &mTexWidth, &mTexHeight, &mNumberOfChannels, 0);
    /*
    // 1. Читаем зашифрованную текстуру в память
    std::vector<char> fileData = Tools::loadFileToMemory(textureFilename);
    if (fileData.empty()) {
        Logger::log(1, "%s error: could not read texture to memory '%s'\n", __FUNCTION__, textureFilename.c_str());
        return false;
    }

    // 2. Расшифровываем
    Tools::processData(fileData);

    // 3. Отдаем расшифрованный массив в stb_image
    // stbi_load_from_memory принимает массив байтов
    unsigned char* textureData = stbi_load_from_memory(
        reinterpret_cast<const stbi_uc*>(fileData.data()),
        static_cast<int>(fileData.size()),
        &mTexWidth, &mTexHeight, &mNumberOfChannels, 0
    );
    */
  
    if (!textureData) {
        Logger::log(1, "%s error: could not load file '%s'\n", __FUNCTION__, mTextureName.c_str());
        stbi_image_free(textureData);
        return false;
    }

    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);

    // ... (твои параметры фильтрации) ...

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    printf("[TEXTURE DEBUG] Attempting to upload: %s (%dx%d)\n", mTextureName.c_str(), mTexWidth, mTexHeight);

    GLenum format;
    GLenum internalFormat;

    if (mNumberOfChannels == 1) {
        format = GL_RED;
        internalFormat = GL_R8;

        // --- МАГИЯ SWIZZLE ---
        // Размножаем красный канал во все остальные, чтобы шейдер (.g) не читал нули
        GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

        printf("  [FORMAT] Loading as Grayscale (GL_RED)\n");
    }
    else if (mNumberOfChannels == 3) {
        format = GL_RGB;
        internalFormat = GL_RGB8;
        printf("  [FORMAT] Loading as RGB\n");
    }
    else {
        format = GL_RGBA;
        internalFormat = GL_RGBA8;
        printf("  [FORMAT] Loading as RGBA\n");
    }

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, mTexWidth, mTexHeight, 0, format, GL_UNSIGNED_BYTE, textureData);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

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

  stbi_image_free(textureData);

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
  

  Logger::log(1, "%s: texture '%s' loaded (%dx%d, %d channels)\n", __FUNCTION__, mTextureName.c_str(), mTexWidth, mTexHeight, mNumberOfChannels);
  return true;
}

void Texture::bind() {
  glBindTexture(GL_TEXTURE_2D, mTexture);
}

void Texture::unbind() {
  glBindTexture(GL_TEXTURE_2D, 0);
}
