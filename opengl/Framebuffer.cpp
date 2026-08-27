#define GLM_ENABLE_EXPERIMENTAL
#include "Framebuffer.h"
#include "Logger.h"


bool Framebuffer::init(unsigned int width, unsigned int height) {
    mBufferWidth = width;
    mBufferHeight = height;

    glGenFramebuffers(1, &mBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, mBuffer);


        glGenTextures(1, &mColorTex);
    glBindTexture(GL_TEXTURE_2D, mColorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorTex, 0);
    Logger::log(1, "%s: added HDR color buffer (COLOR0)\n", __FUNCTION__);

        glGenTextures(1, &mEmissionTex);
    glBindTexture(GL_TEXTURE_2D, mEmissionTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mEmissionTex, 0);
    Logger::log(1, "%s: added emission buffer (COLOR1)\n", __FUNCTION__);

        GLenum drawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, drawBuffers);

        glGenRenderbuffers(1, &mDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, mDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    Logger::log(1, "%s: added depth renderbuffer\n", __FUNCTION__);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return checkComplete();
}

void Framebuffer::cleanup() {
    unbind();
    glDeleteTextures(1, &mColorTex);
    glDeleteTextures(1, &mEmissionTex);
    glDeleteRenderbuffers(1, &mDepthBuffer);
    glDeleteFramebuffers(1, &mBuffer);
}

bool Framebuffer::resize(unsigned int newWidth, unsigned int newHeight) {
    Logger::log(1, "%s: resizing framebuffer from %dx%d to %dx%d\n",
        __FUNCTION__, mBufferWidth, mBufferHeight, newWidth, newHeight);
    mBufferWidth = newWidth;
    mBufferHeight = newHeight;

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDeleteTextures(1, &mColorTex);
    glDeleteTextures(1, &mEmissionTex);
    glDeleteRenderbuffers(1, &mDepthBuffer);
    glDeleteFramebuffers(1, &mBuffer);

    return init(newWidth, newHeight);
}

void Framebuffer::bind() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mBuffer);
}

void Framebuffer::unbind() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Framebuffer::drawToScreen() {
    }

bool Framebuffer::checkComplete() {
    glBindFramebuffer(GL_FRAMEBUFFER, mBuffer);
    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (result != GL_FRAMEBUFFER_COMPLETE) {
        Logger::log(1, "%s error: framebuffer is NOT complete\n", __FUNCTION__);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Logger::log(1, "%s: framebuffer is complete\n", __FUNCTION__);
    return true;
}



bool BlurFramebuffer::init(unsigned int width, unsigned int height) {
    mBufferWidth = width;
    mBufferHeight = height;

    glGenFramebuffers(1, &mBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, mBuffer);

    glGenTextures(1, &mColorTex);
    glBindTexture(GL_TEXTURE_2D, mColorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorTex, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return checkComplete();
}

void BlurFramebuffer::cleanup() {
    unbind();
    glDeleteTextures(1, &mColorTex);
    glDeleteFramebuffers(1, &mBuffer);
}

bool BlurFramebuffer::resize(unsigned int newWidth, unsigned int newHeight) {
    mBufferWidth = newWidth;
    mBufferHeight = newHeight;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDeleteTextures(1, &mColorTex);
    glDeleteFramebuffers(1, &mBuffer);
    return init(newWidth, newHeight);
}

void BlurFramebuffer::bind() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mBuffer);
}

void BlurFramebuffer::unbind() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

bool BlurFramebuffer::checkComplete() {
    glBindFramebuffer(GL_FRAMEBUFFER, mBuffer);
    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (result != GL_FRAMEBUFFER_COMPLETE) {
        Logger::log(1, "%s error: blur framebuffer is NOT complete\n", __FUNCTION__);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Logger::log(1, "%s: blur framebuffer is complete\n", __FUNCTION__);
    return true;
}



bool MSFramebuffer::init(unsigned int width, unsigned int height, int samples) {
    mBufferWidth = width;
    mBufferHeight = height;
    mSamples = samples;

    glGenFramebuffers(1, &mBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, mBuffer);

        glGenRenderbuffers(1, &mColorRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, mColorRBO);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, mSamples, GL_RGBA16F, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mColorRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    Logger::log(1, "%s: added multisample HDR color buffer (COLOR0, %dx MSAA)\n", __FUNCTION__, mSamples);

        glGenRenderbuffers(1, &mEmissionRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, mEmissionRBO);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, mSamples, GL_RGBA16F, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, mEmissionRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    Logger::log(1, "%s: added multisample emission buffer (COLOR1, %dx MSAA)\n", __FUNCTION__, mSamples);

    GLenum drawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, drawBuffers);

        glGenRenderbuffers(1, &mDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, mDepthRBO);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, mSamples, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    Logger::log(1, "%s: added multisample depth renderbuffer\n", __FUNCTION__);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return checkComplete();
}

void MSFramebuffer::cleanup() {
    unbind();
    if (mColorRBO) glDeleteRenderbuffers(1, &mColorRBO);
    if (mEmissionRBO) glDeleteRenderbuffers(1, &mEmissionRBO);
    if (mDepthRBO) glDeleteRenderbuffers(1, &mDepthRBO);
    if (mBuffer) glDeleteFramebuffers(1, &mBuffer);
    mColorRBO = mEmissionRBO = mDepthRBO = mBuffer = 0;
}

bool MSFramebuffer::resize(unsigned int newWidth, unsigned int newHeight) {
    Logger::log(1, "%s: resizing MS framebuffer from %dx%d to %dx%d\n",
        __FUNCTION__, mBufferWidth, mBufferHeight, newWidth, newHeight);
    cleanup();
    return init(newWidth, newHeight, mSamples);
}

bool MSFramebuffer::changeSampleCount(int samples) {
    if (samples == mSamples && mBuffer != 0) {
        return true;     }
    Logger::log(1, "%s: changing MSAA sample count from %d to %d\n", __FUNCTION__, mSamples, samples);
    cleanup();
    return init(mBufferWidth, mBufferHeight, samples);
}

void MSFramebuffer::bind() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mBuffer);
}

void MSFramebuffer::unbind() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void MSFramebuffer::resolveTo(Framebuffer& target) {
            for (int i = 0; i < 2; ++i) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, mBuffer);
        glReadBuffer(GL_COLOR_ATTACHMENT0 + i);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.getFBOHandle());
        glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);

        glBlitFramebuffer(0, 0, mBufferWidth, mBufferHeight,
            0, 0, mBufferWidth, mBufferHeight,
            GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

        glBindFramebuffer(GL_READ_FRAMEBUFFER, mBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.getFBOHandle());
    glBlitFramebuffer(0, 0, mBufferWidth, mBufferHeight,
        0, 0, mBufferWidth, mBufferHeight,
        GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool MSFramebuffer::checkComplete() {
    glBindFramebuffer(GL_FRAMEBUFFER, mBuffer);
    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (result != GL_FRAMEBUFFER_COMPLETE) {
        Logger::log(1, "%s error: MS framebuffer is NOT complete\n", __FUNCTION__);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Logger::log(1, "%s: MS framebuffer is complete\n", __FUNCTION__);
    return true;
}