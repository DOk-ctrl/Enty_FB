#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Результат фоновой загрузки — просто данные в RAM, никакого GL
struct TextureLoadResult {
    bool success = false;
    unsigned char* pixels = nullptr; // нужно освободить через stbi_image_free после аплоада
    int width = 0;
    int height = 0;
    int channels = 0;
    std::string name;
};

class Texture {
public:
    // 🟧 Можно звать из std::async — не трогает OpenGL
    static TextureLoadResult loadTextureData(std::string textureFilename, bool flipImage = true);

    // 🛑 Только главный поток — берёт готовые пиксели и заливает в GPU
    bool uploadToGPU(TextureLoadResult& result);

    // старый метод — можно оставить как synchronous-обёртку для мест, где async не нужен
    bool loadTexture(std::string textureFilename, bool flipImage = true);

    void bind();
    void unbind();
    void cleanup();
    void setMipmapLevel(int level);
    void setAnisotropy(float amount);
private:
    GLuint mTexture = 0;
    int mTexWidth = 0;
    int mTexHeight = 0;
    int mNumberOfChannels = 0;
    std::string mTextureName;
};
