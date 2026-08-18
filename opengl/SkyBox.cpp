#include <cmath>
#include <cstring>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include "SkyBox.h"
#include "Logger.h"
#include <stb_image.h>

/* ============================================================
 *  Geometry — unit cube, shared by all layers
 * ============================================================ */
namespace {
    const float skyboxVertices[] = {
      -1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
       1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,

      -1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,

       1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,

      -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,

      -1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f,

      -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f, -1.0f,
       1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f
    };
}

/* ============================================================
 *  init()  — upload geometry + load shaders
 * ============================================================ */
bool SkyBox::init() {
    /* background shader (same as before, no extra uniforms) */
    if (!mBgShader.loadShaders("shader/skybox.vert", "shader/skybox.frag")) {
        Logger::log(1, "%s error: could not load background skybox shaders\n", __FUNCTION__);
        return false;
    }

    /* parallax layer shader */
    if (!mLayerShader.loadShaders("shader/skybox_parallax.vert", "shader/skybox_parallax.frag")) {
        Logger::log(1, "%s error: could not load parallax skybox shaders\n", __FUNCTION__);
        return false;
    }

    glGenVertexArrays(1, &mVAO);
    glGenBuffers(1, &mVBO);

    glBindVertexArray(mVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    return true;
}

/* ============================================================
 *  loadCrossToCubemap()  — shared PNG→cubemap helper
 * ============================================================ */
bool SkyBox::loadCrossToCubemap(const std::string& filename, GLuint& outTexture,
    bool forceRGBA, bool& outHasRealAlpha) {
    /* Detect actual channel count BEFORE loading (no pixel allocation).
     * If the PNG is RGB-only, forceRGBA fills alpha=255 → all pixels opaque.
     * The caller uses outHasRealAlpha=false to enable brightness-based keying. */
        {
            int iw = 0, ih = 0, ich = 0;
            outHasRealAlpha = stbi_info(filename.c_str(), &iw, &ih, &ich) && (ich == 4);
            Logger::log(1, "%s: '%s' real alpha: %s\n",
                __FUNCTION__, filename.c_str(), outHasRealAlpha ? "yes" : "NO — brightness key will be applied");
        }

        int texWidth = 0, texHeight = 0, texChannels = 0;
        stbi_set_flip_vertically_on_load(false);

        int reqChannels = forceRGBA ? 4 : 0;
        unsigned char* crossData = stbi_load(filename.c_str(), &texWidth, &texHeight,
            &texChannels, reqChannels);
        if (!crossData) {
            Logger::log(1, "%s error: could not load image '%s'\n", __FUNCTION__, filename.c_str());
            return false;
        }
        if (forceRGBA) texChannels = 4;

        int faceSize = texWidth / 4;
        if (texHeight / 3 != faceSize) {
            Logger::log(1, "%s error: '%s' is not a 4x3 cross image (%dx%d)\n",
                __FUNCTION__, filename.c_str(), texWidth, texHeight);
            stbi_image_free(crossData);
            return false;
        }

        /* cross layout:
         *        [ ][+Y][ ][ ]     row 0
         *     [-X][+Z][+X][-Z]     row 1
         *        [ ][-Y][ ][ ]     row 2   */
        struct FacePos { int col, row; };
        const FacePos facePositions[6] = {
          { 2, 1 }, /* +X */
          { 0, 1 }, /* -X */
          { 1, 0 }, /* +Y */
          { 1, 2 }, /* -Y */
          { 1, 1 }, /* +Z */
          { 3, 1 }  /* -Z */
        };

        std::vector<unsigned char> faceBuffer(static_cast<size_t>(faceSize) * faceSize * texChannels);
        GLenum format = (texChannels == 4) ? GL_RGBA : GL_RGB;
        // Указываем внутренний формат с поддержкой аппаратного sRGB-декодирования
        GLenum internalFormat = (texChannels == 4) ? GL_SRGB_ALPHA : GL_SRGB8;

        glGenTextures(1, &outTexture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, outTexture);

        for (int face = 0; face < 6; ++face) {
            int startX = facePositions[face].col * faceSize;
            int startY = facePositions[face].row * faceSize;

            for (int row = 0; row < faceSize; ++row) {
                const unsigned char* src = crossData +
                    (static_cast<size_t>(startY + row) * texWidth + startX) * texChannels;
                unsigned char* dst = faceBuffer.data() +
                    static_cast<size_t>(row) * faceSize * texChannels;
                std::memcpy(dst, src, static_cast<size_t>(faceSize) * texChannels);
            }

            // Заменяем первый format на internalFormat
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, internalFormat,
                faceSize, faceSize, 0, format, GL_UNSIGNED_BYTE, faceBuffer.data());
        }

        stbi_image_free(crossData);

        /* mipmaps needed by IBL prefilter pass (background layer only, but harmless elsewhere) */
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        Logger::log(1, "%s: loaded cubemap '%s' (face %dx%d, ch=%d)\n",
            __FUNCTION__, filename.c_str(), faceSize, faceSize, texChannels);
        return true;
}

/* ============================================================
 *  loadCrossCubemap()  — background / IBL source  (slot 0)
 * ============================================================ */
bool SkyBox::loadCrossCubemap(std::string crossImageFilename) {
    bool dummy = false;
    if (!loadCrossToCubemap(crossImageFilename, mLayers[0].texture, false, dummy)) {
        return false;
    }
    mLayers[0].parallaxFactor = 0.0f;
    mLayers[0].windSpeed = 0.0f;
    mLayers[0].hasAlpha = false;
    mLayerCount = std::max(mLayerCount, 1);
    return true;
}

/* ============================================================
 *  addParallaxLayer()  — foreground layers (slots 1 and 2)
 * ============================================================ */
bool SkyBox::addParallaxLayer(int slot, std::string crossImageFilename,
    float parallaxFactor,
    float windSpeed,
    glm::vec2 windDir,
    float opacity,
    float brightnessKey) {
    if (slot < 1 || slot >= SKY_MAX_LAYERS) {
        Logger::log(1, "%s error: slot %d out of range [1, %d)\n",
            __FUNCTION__, slot, SKY_MAX_LAYERS);
        return false;
    }

    bool hasRealAlpha = false;
    if (!loadCrossToCubemap(crossImageFilename, mLayers[slot].texture, true, hasRealAlpha)) {
        return false;
    }
    mLayers[slot].parallaxFactor = parallaxFactor;
    mLayers[slot].windSpeed = windSpeed;
    mLayers[slot].windDir = glm::normalize(windDir);
    mLayers[slot].opacity = opacity;
    mLayers[slot].hasAlpha = true;
    mLayers[slot].hasRealAlpha = hasRealAlpha;
    mLayers[slot].brightnessKey = brightnessKey;
    mLayerCount = std::max(mLayerCount, slot + 1);
    return true;
}

/* ============================================================
 *  draw()
 * ============================================================ */
void SkyBox::draw(glm::mat4 view, glm::mat4 projection, float cameraYaw, float time) {
    glDepthFunc(GL_LEQUAL);
    glBindVertexArray(mVAO);

    for (int i = 0; i < mLayerCount; ++i) {
        const SkyLayer& layer = mLayers[i];
        if (layer.texture == 0) continue;

        if (layer.hasAlpha) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        else {
            glDisable(GL_BLEND);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, layer.texture);

        if (i == 0) {
            /* ---- background: standard shader, no parallax ---- */
            mBgShader.use();
            if (mBgShader.getUniformLocation("view"))       mBgShader.setUniformValue(view);
            if (mBgShader.getUniformLocation("projection")) mBgShader.setUniformValue(projection);
            if (mBgShader.getUniformLocation("skybox"))     mBgShader.setUniformValue(0);

        }
        else {
            /* ---- parallax layer ---- */

            /* Parallax: small extra Y-rotation so this layer appears "closer"
             * than the background. Proportional to how far camera has rotated. */
            float parallaxAngle = layer.parallaxFactor * cameraYaw;

            /* Wind: LINEAR XZ drift.
             * Wraps with fmod every 2π units so the offset never grows huge
             * and cubemap face boundaries loop cleanly (no visible seam). */
            const float TAU = 6.28318530718f;
            float windOffset = std::fmod(time * layer.windSpeed, TAU);
            glm::vec2 windDrift = layer.windDir * windOffset;

            mLayerShader.use();
            if (mLayerShader.getUniformLocation("view"))           mLayerShader.setUniformValue(view);
            if (mLayerShader.getUniformLocation("projection"))     mLayerShader.setUniformValue(projection);
            if (mLayerShader.getUniformLocation("skybox"))         mLayerShader.setUniformValue(0);
            if (mLayerShader.getUniformLocation("u_ParallaxAngle"))  mLayerShader.setUniformValue(parallaxAngle);
            if (mLayerShader.getUniformLocation("u_WindDrift"))      mLayerShader.setUniformValue(windDrift);
            if (mLayerShader.getUniformLocation("u_Opacity"))        mLayerShader.setUniformValue(layer.opacity);
            if (mLayerShader.getUniformLocation("u_HasRealAlpha"))   mLayerShader.setUniformValue(layer.hasRealAlpha ? 1 : 0);
            if (mLayerShader.getUniformLocation("u_BrightnessKey"))  mLayerShader.setUniformValue(layer.brightnessKey);
        }

        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    glDisable(GL_BLEND);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

/* ============================================================
 *  IBL — identical to previous version
 * ============================================================ */
bool SkyBox::initIBLShaders() {
    if (!mIrradianceShader.loadShaders("shader/ibl_irradiance.vert", "shader/ibl_irradiance.frag")) {
        Logger::log(1, "%s error: could not load irradiance shader\n", __FUNCTION__);
        return false;
    }
    if (!mPrefilterShader.loadShaders("shader/ibl_prefilter.vert", "shader/ibl_prefilter.frag")) {
        Logger::log(1, "%s error: could not load prefilter shader\n", __FUNCTION__);
        return false;
    }
    if (!mBRDFShader.loadShaders("shader/ibl_brdf.vert", "shader/ibl_brdf.frag")) {
        Logger::log(1, "%s error: could not load BRDF LUT shader\n", __FUNCTION__);
        return false;
    }
    return true;
}

bool SkyBox::generateIBLMaps() {
    if (!initIBLShaders()) return false;

    float quadVertices[] = {
      -1.0f,  1.0f,  0.0f, 1.0f,
      -1.0f, -1.0f,  0.0f, 0.0f,
       1.0f,  1.0f,  1.0f, 1.0f,
       1.0f, -1.0f,  1.0f, 0.0f,
    };
    glGenVertexArrays(1, &mQuadVAO);
    glGenBuffers(1, &mQuadVBO);
    glBindVertexArray(mQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    glGenFramebuffers(1, &mCaptureFBO);
    glGenRenderbuffers(1, &mCaptureRBO);

    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
      glm::lookAt(glm::vec3(0), glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
      glm::lookAt(glm::vec3(0), glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
      glm::lookAt(glm::vec3(0), glm::vec3(0, 1, 0), glm::vec3(0,  0, 1)),
      glm::lookAt(glm::vec3(0), glm::vec3(0,-1, 0), glm::vec3(0,  0,-1)),
      glm::lookAt(glm::vec3(0), glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
      glm::lookAt(glm::vec3(0), glm::vec3(0, 0,-1), glm::vec3(0, -1, 0)),
    };

    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);
    glBindFramebuffer(GL_FRAMEBUFFER, mCaptureFBO);

    /* --- 1. Irradiance --- */
    glGenTextures(1, &mIrradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mIrradianceMap);
    for (int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
            mIrradianceMapSize, mIrradianceMapSize, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindRenderbuffer(GL_RENDERBUFFER, mCaptureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
        mIrradianceMapSize, mIrradianceMapSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mCaptureRBO);

    mIrradianceShader.use();
    if (mIrradianceShader.getUniformLocation("environmentMap")) mIrradianceShader.setUniformValue(0);
    if (mIrradianceShader.getUniformLocation("projection"))     mIrradianceShader.setUniformValue(captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mLayers[0].texture);
    glViewport(0, 0, mIrradianceMapSize, mIrradianceMapSize);
    glBindVertexArray(mVAO);
    for (int i = 0; i < 6; ++i) {
        if (mIrradianceShader.getUniformLocation("view")) mIrradianceShader.setUniformValue(captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mIrradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    /* --- 2. Prefilter --- */
    glGenTextures(1, &mPrefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mPrefilterMap);
    for (int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
            mPrefilterMapSize, mPrefilterMapSize, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    mPrefilterShader.use();
    if (mPrefilterShader.getUniformLocation("environmentMap")) mPrefilterShader.setUniformValue(0);
    if (mPrefilterShader.getUniformLocation("projection"))     mPrefilterShader.setUniformValue(captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mLayers[0].texture);

    for (int mip = 0; mip < mPrefilterMipLevels; ++mip) {
        int mipSize = static_cast<int>(mPrefilterMapSize * std::pow(0.5, mip));
        glBindRenderbuffer(GL_RENDERBUFFER, mCaptureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipSize, mipSize);
        glViewport(0, 0, mipSize, mipSize);
        float roughness = static_cast<float>(mip) / static_cast<float>(mPrefilterMipLevels - 1);
        if (mPrefilterShader.getUniformLocation("roughness")) mPrefilterShader.setUniformValue(roughness);
        for (int i = 0; i < 6; ++i) {
            if (mPrefilterShader.getUniformLocation("view")) mPrefilterShader.setUniformValue(captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mPrefilterMap, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
    glBindVertexArray(0);

    /* --- 3. BRDF LUT --- */
    glGenTextures(1, &mBRDFLUTMap);
    glBindTexture(GL_TEXTURE_2D, mBRDFLUTMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, mBRDFMapSize, mBRDFMapSize, 0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindRenderbuffer(GL_RENDERBUFFER, mCaptureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mBRDFMapSize, mBRDFMapSize);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mBRDFLUTMap, 0);
    glViewport(0, 0, mBRDFMapSize, mBRDFMapSize);
    mBRDFShader.use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(mQuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);

    Logger::log(1, "%s: IBL maps generated (irradiance %d, prefilter %d x %d mips, LUT %d)\n",
        __FUNCTION__, mIrradianceMapSize, mPrefilterMapSize, mPrefilterMipLevels, mBRDFMapSize);
    return true;
}

/* ============================================================
 *  cleanup()
 * ============================================================ */
void SkyBox::cleanup() {
    mBgShader.cleanup();
    mLayerShader.cleanup();
    mIrradianceShader.cleanup();
    mPrefilterShader.cleanup();
    mBRDFShader.cleanup();

    glDeleteVertexArrays(1, &mVAO);
    glDeleteBuffers(1, &mVBO);
    glDeleteVertexArrays(1, &mQuadVAO);
    glDeleteBuffers(1, &mQuadVBO);

    glDeleteFramebuffers(1, &mCaptureFBO);
    glDeleteRenderbuffers(1, &mCaptureRBO);

    for (auto& layer : mLayers) {
        if (layer.texture) glDeleteTextures(1, &layer.texture);
    }
    glDeleteTextures(1, &mIrradianceMap);
    glDeleteTextures(1, &mPrefilterMap);
    glDeleteTextures(1, &mBRDFLUTMap);
}