#pragma once

#include <array>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Shader.h"
static constexpr int SKY_MAX_LAYERS = 3;

struct SkyLayer {
    GLuint      texture = 0;
    float       parallaxFactor = 0.0f;
    float       windSpeed = 0.0f;
    glm::vec2   windDir = glm::vec2(0.0f, 1.0f);
    float       opacity = 1.0f;
    bool        hasAlpha = false;

    bool        hasRealAlpha = false;
    float       brightnessKey = 0.75f;
};

class SkyBox {
public:
    
    bool init();


    bool loadCrossCubemap(std::string crossImageFilename);

    bool addParallaxLayer(int slot, std::string crossImageFilename,
        float parallaxFactor,
        float windSpeed = 0.0f,
        glm::vec2 windDir = glm::vec2(0.0f, 1.0f),
        float opacity = 1.0f,
        float brightnessKey = 0.75f);

    
    bool generateIBLMaps();


    void draw(glm::mat4 view, glm::mat4 projection, float cameraYaw, float time);

    
    GLuint getEnvironmentMap()  const { return mLayers[0].texture; }
    GLuint getIrradianceMap()   const { return mIrradianceMap; }
    GLuint getPrefilterMap()    const { return mPrefilterMap; }
    GLuint getBRDFLUTMap()      const { return mBRDFLUTMap; }
    int    getPrefilterMipLevels() const { return mPrefilterMipLevels; }

    void cleanup();

private:
 
    bool loadCrossToCubemap(const std::string& filename, GLuint& outTexture,
        bool forceRGBA, bool& outHasRealAlpha);

    bool initIBLShaders();

    
    GLuint mVAO = 0;
    GLuint mVBO = 0;

    
    std::array<SkyLayer, SKY_MAX_LAYERS> mLayers{};
    int mLayerCount = 0;

    
    GLuint mIrradianceMap = 0;
    GLuint mPrefilterMap = 0;
    GLuint mBRDFLUTMap = 0;
    GLuint mCaptureFBO = 0;
    GLuint mCaptureRBO = 0;
    GLuint mQuadVAO = 0;
    GLuint mQuadVBO = 0;

    
    Shader mBgShader{};        
    Shader mLayerShader{};     
    Shader mIrradianceShader{};
    Shader mPrefilterShader{};
    Shader mBRDFShader{};

    static const int mIrradianceMapSize = 32;
    static const int mPrefilterMapSize = 128;
    static const int mPrefilterMipLevels = 5;
    static const int mBRDFMapSize = 512;
};