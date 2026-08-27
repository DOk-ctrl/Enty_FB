#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GameStateMachine.h"
#include "JigglePhysics.h" 
struct OGLRenderData;
class GltfModel;
class GltfInstance;

enum class MenuAnimState {
    HIPHOP,          
    CROSSFADE_IDLE,  
    IDLE,            
    CROSSFADE_HH,    
    SALUTE           
};

class MenuController {
public:

    bool init(std::shared_ptr<GltfModel> model);

    MenuAction update(float deltaTime, bool anyButtonHovered);
    MenuAction drawCreditsUI(OGLRenderData& renderData);

    MenuAction drawUI(OGLRenderData& renderData);


    glm::mat4 getCameraViewMatrix() const;

    
    std::shared_ptr<GltfInstance> getInstance() const { return mNJInstance; }

    
    bool isSaluteFinished() const { return mSaluteFinished; }

    static constexpr float ROOM_X = 0.0f;
    static constexpr float ROOM_Y = 0.0f;
    static constexpr float ROOM_Z = 0.0f;
    inline static const glm::vec3 NJ_WORLD_POS = glm::vec3(ROOM_X, ROOM_Y, ROOM_Z);

    void cleanup();
   
private:
    
    void startCrossfade(int fromClip, int toClip);
    void tickCrossfade(float dt);
    void startSalute();
    void tickSalute(float dt);
    void applySettings();
    BreastJiggle mLeftBreastJiggleNJ;
    BreastJiggle mRightBreastJiggleNJ;
    
    std::shared_ptr<GltfModel>    mNJModel;
    std::shared_ptr<GltfInstance> mNJInstance;

    MenuAnimState mAnimState = MenuAnimState::HIPHOP;

    
    int   mBlendSrcClip = 0;
    int   mBlendDstClip = 0;
    float mBlendTimer = 0.0f;
    float mBlendDuration = 1.0f;   
    float mBlendFactor = 1.0f;    

    
    float mSaluteTimer = 0.0f;
    float mSaluteDuration = 0.0f;
    bool  mSaluteFinished = false;

    
    int   mCurrentClip = 0;


    static constexpr int ANIM_HIPHOP = 1;
    static constexpr int ANIM_IDLE = 2;
    static constexpr int ANIM_SALUTE = 3;

    
   
    const glm::vec3 NJ_WORLD_ROT = glm::vec3(0.0f, 30.0f, 0.0f);

    
    const glm::vec3 CAM_OFFSET = glm::vec3(1.8f, 1.6f, 2.5f);
    const glm::vec3 CAM_LOOK_OFFSET = glm::vec3(0.0f, 1.8f, 0.0f);
};