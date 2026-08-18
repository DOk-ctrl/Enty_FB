#define GLM_ENABLE_EXPERIMENTAL
#include <stb_image.h>
#include<vector>
#include "Tools.h"
#include "Texture.h"
#include "Logger.h"

void Texture::cleanup() {
    glDeleteTextures(1, &mTexture);
}

void Texture::setMipmapLevel(int level)
{
    if (mTexture == 0) return; // Защита от пустых текстур

    glBindTexture(GL_TEXTURE_2D, mTexture);
    // Вот она, наша магическая строчка!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, level);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::setAnisotropy(float amount)
{
    if (mTexture == 0) return; // текстура ещё не загружена

    static GLfloat maxSupported = -1.0f;
    if (maxSupported < 0.0f) {
        // GL_TEXTURE_MAX_ANISOTROPY / GL_MAX_TEXTURE_MAX_ANISOTROPY входят
        // в ядро начиная с OpenGL 4.6 (тот же код енама, что у *_EXT версий),
        // так что отдельный EXT-путь для контекста 4.6 core не нужен.
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxSupported);
    }

    float clamped = std::min(amount, maxSupported);

    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, clamped);
    glBindTexture(GL_TEXTURE_2D, 0);

    Logger::log(1, "%s: texture '%s' anisotropy set to %.1fx (max supported %.1fx)\n",
        __FUNCTION__, mTextureName.c_str(), clamped, maxSupported);
}

bool Texture::loadTexture(std::string textureFilename, bool flipImage) {
    mTextureName = textureFilename;

    // СБРАСЫВАЕМ все накопившиеся ошибки OpenGL до начала загрузки!
    while (glGetError() != GL_NO_ERROR);

    stbi_set_flip_vertically_on_load(flipImage);

    int actualChannelsInFile = 0;
    unsigned char* textureData = stbi_load(textureFilename.c_str(), &mTexWidth, &mTexHeight, &actualChannelsInFile, STBI_rgb_alpha);

    if (!textureData) {
        Logger::log(1, "%s ERROR: stbi_load failed for '%s'\n", __FUNCTION__, mTextureName.c_str());
        return false;
    }

    mNumberOfChannels = 4;

    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    printf("[TEXTURE DEBUG] Loading: %s | Size: %dx%d | Original Channels: %d\n",
        mTextureName.c_str(), mTexWidth, mTexHeight, actualChannelsInFile);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mTexWidth, mTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);

    GLenum uploadErr = glGetError();
    if (uploadErr != GL_NO_ERROR) {
        printf("!!! [TEXTURE ERROR] glTexImage2D FAILED code: 0x%X (Width: %d, Height: %d)\n",
            uploadErr, mTexWidth, mTexHeight);
    }
    else {
        printf("  glTexImage2D: OK\n");
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D);

    GLenum mipErr = glGetError();
    if (mipErr != GL_NO_ERROR) {
        printf("!!! [TEXTURE ERROR] glGenerateMipmap FAILED code: 0x%X\n", mipErr);
    }
    else {
        printf("  glGenerateMipmap: OK\n");
    }

    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);

    return (uploadErr == GL_NO_ERROR && mipErr == GL_NO_ERROR);
}

void Texture::bind() {
    glBindTexture(GL_TEXTURE_2D, mTexture);
}

void Texture::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}
