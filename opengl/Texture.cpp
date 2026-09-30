#define GLM_ENABLE_EXPERIMENTAL
#include <stb_image.h>
#include<vector>
#include "Tools.h"
#include "Texture.h"
#include "Logger.h"

#include <sstream>
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
// 🟧 Можно звать из std::async — не трогает OpenGL
TextureLoadResult Texture::loadTextureData(std::string textureFilename, bool flipImage)
{
    TextureLoadResult result;
    result.name = textureFilename;

    stbi_set_flip_vertically_on_load(flipImage);

    int actualChannelsInFile = 0;
    result.pixels = stbi_load(textureFilename.c_str(), &result.width, &result.height,
        &actualChannelsInFile, STBI_rgb_alpha);

    if (!result.pixels) {
        Logger::log(1, "%s ERROR: stbi_load failed for '%s'\n", __FUNCTION__, textureFilename.c_str());
        result.success = false;
        return result;
    }

    result.channels = 4; // STBI_rgb_alpha всегда даёт 4 канала
    result.success = true;
    return result;
}
// 🛑 Только главный поток — берёт готовые пиксели и заливает в GPU
bool Texture::uploadToGPU(TextureLoadResult& result)
{
    if (!result.success || !result.pixels) return false;

    mTextureName = result.name;
    mTexWidth = result.width;
    mTexHeight = result.height;
    mNumberOfChannels = result.channels;

    while (glGetError() != GL_NO_ERROR); // сброс накопившихся ошибок

    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mTexWidth, mTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, result.pixels);

    GLenum uploadErr = glGetError();
    if (uploadErr != GL_NO_ERROR) {
        Logger::log(1, "%s ERROR: glTexImage2D failed 0x%X for '%s'\n", __FUNCTION__, uploadErr, mTextureName.c_str());
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D);

    GLenum mipErr = glGetError();
    if (mipErr != GL_NO_ERROR) {
        Logger::log(1, "%s ERROR: glGenerateMipmap failed 0x%X\n", __FUNCTION__, mipErr);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    // Пиксели больше не нужны — освобождаем ПОСЛЕ аплоада
    stbi_image_free(result.pixels);
    result.pixels = nullptr;

    return (uploadErr == GL_NO_ERROR && mipErr == GL_NO_ERROR);
}

bool Texture::loadTexture(std::string textureFilename, bool flipImage) {

    TextureLoadResult result = loadTextureData(textureFilename, flipImage);

    return uploadToGPU(result);

   /* mTextureName = textureFilename;

    // СБРАСЫВАЕМ все накопившиеся ошибки OpenGL до начала загрузки!
    while (glGetError() != GL_NO_ERROR);
    stbi_set_flip_vertically_on_load(flipImage);
    int actualChannelsInFile = 0;

    // 1. Читаем зашифрованную текстуру в память
    std::vector<char> fileData = Tools::loadFileToMemory(textureFilename);
    if (fileData.empty()) {
        Logger::log(1, "%s error: could not read texture to memory '%s'\n", __FUNCTION__, textureFilename.c_str());
        return false;
    }

    // 2. Расшифровываем
    Tools::processData(fileData, TEXTURE_KEY);

    // 3. Отдаём расшифрованный массив в stb_image
    unsigned char* textureData = stbi_load_from_memory(
        reinterpret_cast<const stbi_uc*>(fileData.data()),
        static_cast<int>(fileData.size()),
        &mTexWidth, &mTexHeight, &actualChannelsInFile, 4
    );

    if (!textureData) {
        Logger::log(1, "%s ERROR: stbi_load failed for '%s': %s\n",
            __FUNCTION__, mTextureName.c_str(), stbi_failure_reason());
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
    */

}

void Texture::bind() {
    glBindTexture(GL_TEXTURE_2D, mTexture);
}

void Texture::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}
