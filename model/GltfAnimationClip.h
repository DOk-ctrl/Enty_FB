/* glTF animation clip */
#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <string>
#include <vector>
#include <memory>
#include <tiny_gltf.h>

#include "GltfNode.h"
#include "GltfAnimationChannel.h"

class GltfAnimationClip {
  public:
    GltfAnimationClip(std::string name);
    void addChannel(const std::shared_ptr<tinygltf::Model>& model, const tinygltf::Animation& anim,
      const tinygltf::AnimationChannel& channel);

    void setAnimationFrame(std::vector<std::shared_ptr<GltfNode>>& nodes,
      const std::vector<bool>& additiveMask, float time);
    void blendAnimationFrame(std::vector<std::shared_ptr<GltfNode>> nodes,
      std::vector<bool> additiveMask, float time, float blendFactor);

    void reserveChannels(size_t count) {
        mAnimationChannels.reserve(count);     }

    float getClipEndTime();
    std::string getClipName();

  private:
    std::vector<std::shared_ptr<GltfAnimationChannel>> mAnimationChannels{};

    std::string mClipName;
};
