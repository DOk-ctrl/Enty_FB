#define GLM_ENABLE_EXPERIMENTAL
#include "GltfAnimationClip.h"

GltfAnimationClip::GltfAnimationClip(std::string name) : mClipName(name) {}

void GltfAnimationClip::addChannel(const std::shared_ptr<tinygltf::Model>& model,
    const tinygltf::Animation& anim, const tinygltf::AnimationChannel& channel) {
        auto chan = std::make_shared<GltfAnimationChannel>();
    chan->loadChannelData(model, anim, channel);

        mAnimationChannels.push_back(std::move(chan));
}

void GltfAnimationClip::setAnimationFrame(std::vector<std::shared_ptr<GltfNode>>& nodes,     const std::vector<bool>& additiveMask,           float time) {

        for (const auto& channel : mAnimationChannels) {
        int targetNode = channel->getTargetNode();

                if (targetNode >= 0 && targetNode < additiveMask.size() && additiveMask[targetNode]) {

                        auto& tNode = nodes[targetNode];
            if (!tNode) continue;

            switch (channel->getTargetPath()) {
            case ETargetPath::ROTATION:
                tNode->setRotation(channel->getRotation(time));
                break;

            case ETargetPath::TRANSLATION: {
                glm::vec3 translation = channel->getTranslation(time);

                                                const std::string& nodeName = tNode->getNodeName();
                if (nodeName == "mixamorig:Hips" || nodeName == "Hips") {
                    translation.x = 0.0f;
                    translation.z = 0.0f;
                }

                tNode->setTranslation(translation);
                break;
            }

            case ETargetPath::SCALE:
                tNode->setScale(channel->getScaling(time));
                break;
            }
        }
    }

    /* update all nodes in a single run */
    for (auto& node : nodes) {
        if (node) {
            node->calculateLocalTRSMatrix();
        }
    }
}

void GltfAnimationClip::blendAnimationFrame(std::vector<std::shared_ptr<GltfNode>> nodes,
    std::vector<bool> additiveMask, float time, float blendFactor) {
    for (auto& channel : mAnimationChannels) {
        int targetNode = channel->getTargetNode();
        /* do not change if masked out */
        if (additiveMask.at(targetNode)) {
            switch (channel->getTargetPath()) {
            case ETargetPath::ROTATION:
                nodes.at(targetNode)->blendRotation(channel->getRotation(time), blendFactor);
                break;
            case ETargetPath::TRANSLATION: {                 glm::vec3 translation = channel->getTranslation(time);
                std::shared_ptr<GltfNode> tNode = nodes.at(targetNode);

                                if (tNode->getNodeName() == "mixamorig:Hips" || tNode->getNodeName() == "Hips") {
                    translation.x = 0.0f;
                    translation.z = 0.0f;
                }

                tNode->blendTranslation(translation, blendFactor);
                break;
            }             case ETargetPath::SCALE:
                nodes.at(targetNode)->blendScale(channel->getScaling(time), blendFactor);
                break;
            }
        }
    }
    /* update all nodes in a single run */
    for (auto& node : nodes) {
        if (node) {
            node->calculateLocalTRSMatrix();
        }
    }
}

float GltfAnimationClip::getClipEndTime() {
    return mAnimationChannels.at(0)->getMaxTime();;
}

std::string GltfAnimationClip::getClipName() {
    return mClipName;
}

