#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// -------------------------------------------------------
// Основной FBO: два цветовых аттачмента
//   COLOR0 — обычная HDR-картинка
//   COLOR1 — только emission (идёт в bloom)
// -------------------------------------------------------
class Framebuffer {
public:
    bool init(unsigned int width, unsigned int height);
    bool resize(unsigned int newWidth, unsigned int newHeight);
    void bind();
    void unbind();
    void drawToScreen();
    void cleanup();

    unsigned int getFBOHandle() const { return mBuffer; }
    // Возвращает текстуру основной картинки (COLOR0)
    GLuint getColorTex()    const { return mColorTex; }
    // Возвращает текстуру emission-маски (COLOR1)
    GLuint getEmissionTex() const { return mEmissionTex; }

private:
    unsigned int mBufferWidth = 640;
    unsigned int mBufferHeight = 480;
    GLuint mBuffer = 0;
    GLuint mColorTex = 0;   // COLOR_ATTACHMENT0
    GLuint mEmissionTex = 0;   // COLOR_ATTACHMENT1
    GLuint mDepthBuffer = 0;

    bool checkComplete();
};

// -------------------------------------------------------
// Ping-pong FBO: один цветовой аттачмент, используется
// для горизонтального и вертикального blur-прохода.
// -------------------------------------------------------
class BlurFramebuffer {
public:
    bool init(unsigned int width, unsigned int height);
    bool resize(unsigned int newWidth, unsigned int newHeight);
    void bind();
    void unbind();
    void cleanup();

    GLuint getColorTex() const { return mColorTex; }

private:
    unsigned int mBufferWidth = 640;
    unsigned int mBufferHeight = 480;
    GLuint mBuffer = 0;
    GLuint mColorTex = 0;

    bool checkComplete();
};
class MSFramebuffer {
public:
    bool init(unsigned int width, unsigned int height, int samples);
    void cleanup();
    bool resize(unsigned int newWidth, unsigned int newHeight);
    bool changeSampleCount(int samples);
    void bind();
    void unbind();
    void resolveTo(Framebuffer& target);
    bool checkComplete();

private:
    unsigned int mBuffer = 0;
    unsigned int mColorRBO = 0;
    unsigned int mEmissionRBO = 0;
    unsigned int mDepthRBO = 0;
    unsigned int mBufferWidth = 0;
    unsigned int mBufferHeight = 0;
    int mSamples = 4;
};