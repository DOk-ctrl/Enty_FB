#pragma once

#include <array>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Shader.h"

/* ============================================================
 *  Layered Parallax Skybox
 * ============================================================
 *
 * SLOT LAYOUT:
 *   slot 0 — background  (sky + sun, opaque, no parallax)
 *             also used as the source for IBL maps
 *   slot 1 — mid-ground  (mountains/terrain, PNG with alpha, slow parallax)
 *   slot 2 — foreground  (clouds,            PNG with alpha, fast parallax + wind)
 *
 * PARALLAX TRICK (Dark-Souls-2 style):
 *   All layers are drawn on the same unit cube at the far plane.
 *   Instead of moving the geometry, we rotate the *texture-sample direction*
 *   by an extra angle each frame:
 *
 *     extraAngle = parallaxFactor * cameraYaw  +  time * windSpeed
 *
 *   parallaxFactor == 0   → layer tracks camera 1:1 (normal skybox)
 *   parallaxFactor == 0.2 → layer rotates slightly more than camera → appears closer
 *   parallaxFactor == 0.5 → even more rotation → even closer
 *
 *   cameraYaw is extracted from the view matrix in OGLRenderer and passed to draw().
 *
 * ALL CROSS-FORMAT IMAGES (4 cols x 3 rows):
 *
 *        [ ][+Y][ ][ ]
 *     [-X][+Z][+X][-Z]
 *        [ ][-Y][ ][ ]
 *
 * ============================================================ */

static constexpr int SKY_MAX_LAYERS = 3;

struct SkyLayer {
    GLuint      texture = 0;
    float       parallaxFactor = 0.0f;
    float       windSpeed = 0.0f;
    glm::vec2   windDir = glm::vec2(0.0f, 1.0f);
    float       opacity = 1.0f;
    bool        hasAlpha = false;
    /* --- auto-keying for images without a real alpha channel ---
     * hasRealAlpha: detected automatically by loadCrossToCubemap()
     *               (true = PNG had 4 channels, false = RGB only)
     * brightnessKey: luminance threshold above which pixels become transparent.
     *   0.75 = remove bright sky, keep dark mountain silhouettes
     *   0.60 = more aggressive (removes medium-bright haze too)
     *   1.00 = disabled (keep all pixels regardless of brightness)      */
    bool        hasRealAlpha = false;
    float       brightnessKey = 0.75f;
};

class SkyBox {
public:
    /* --- INITIALISATION --- */
    bool init();

    /* Load the background (slot 0).  Always call this first.
     * The loaded cubemap is also the source for IBL. */
    bool loadCrossCubemap(std::string crossImageFilename);

    /* Add a parallax layer into slot 1 or 2.
     *   parallaxFactor : 0.10–0.20 for mountains, 0.25–0.40 for clouds
     *   windSpeed      : units/s for linear drift; 0.003–0.008 looks natural
     *   windDir        : normalised XZ direction; {0,1} = toward +Z ("forward")
     *   opacity        : 0..1; try 0.9 for mountains, 0.75 for clouds
     *   brightnessKey  : luminance cutoff for auto-keying when PNG has no real alpha.
     *                    0.75 = remove bright sky, keep dark mountain silhouettes.
     *                    1.0  = disabled (image has real alpha channel).             */
    bool addParallaxLayer(int slot, std::string crossImageFilename,
        float parallaxFactor,
        float windSpeed = 0.0f,
        glm::vec2 windDir = glm::vec2(0.0f, 1.0f),
        float opacity = 1.0f,
        float brightnessKey = 0.75f);

    /* --- IBL (unchanged API, still driven by slot-0 cubemap) --- */
    bool generateIBLMaps();

    /* --- RENDERING ---
     *   view        : rotation-only view matrix  (strip translation with mat3 before passing)
     *   projection  : standard perspective matrix
     *   cameraYaw   : world-space yaw of the camera in radians
     *                 extract with: std::atan2(viewMatrix[2][0], viewMatrix[2][2])
     *   time        : accumulated time in seconds (for wind animation)                */
    void draw(glm::mat4 view, glm::mat4 projection, float cameraYaw, float time);

    /* --- IBL MAP ACCESSORS (same as before) --- */
    GLuint getEnvironmentMap()  const { return mLayers[0].texture; }
    GLuint getIrradianceMap()   const { return mIrradianceMap; }
    GLuint getPrefilterMap()    const { return mPrefilterMap; }
    GLuint getBRDFLUTMap()      const { return mBRDFLUTMap; }
    int    getPrefilterMipLevels() const { return mPrefilterMipLevels; }

    void cleanup();

private:
    /* shared helper: loads a cross-format PNG into a new GL_TEXTURE_CUBE_MAP handle.
     * outHasRealAlpha is set to true if the source PNG actually had 4 channels.
     * If the PNG is RGB-only, forceRGBA=true fills alpha with 255 (fully opaque)
     * and outHasRealAlpha is false — the caller should apply brightness keying. */
    bool loadCrossToCubemap(const std::string& filename, GLuint& outTexture,
        bool forceRGBA, bool& outHasRealAlpha);

    bool initIBLShaders();

    /* geometry shared by all layers */
    GLuint mVAO = 0;
    GLuint mVBO = 0;

    /* up to 3 sky layers */
    std::array<SkyLayer, SKY_MAX_LAYERS> mLayers{};
    int mLayerCount = 0;

    /* IBL resources (derived from slot-0 cubemap) */
    GLuint mIrradianceMap = 0;
    GLuint mPrefilterMap = 0;
    GLuint mBRDFLUTMap = 0;
    GLuint mCaptureFBO = 0;
    GLuint mCaptureRBO = 0;
    GLuint mQuadVAO = 0;
    GLuint mQuadVBO = 0;

    /* shaders */
    Shader mBgShader{};        /* skybox.vert / skybox.frag   — background, no parallax */
    Shader mLayerShader{};     /* skybox_parallax.vert / .frag — parallax + alpha layers */
    Shader mIrradianceShader{};
    Shader mPrefilterShader{};
    Shader mBRDFShader{};

    static const int mIrradianceMapSize = 32;
    static const int mPrefilterMapSize = 128;
    static const int mPrefilterMipLevels = 5;
    static const int mBRDFMapSize = 512;
};