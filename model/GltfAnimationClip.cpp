#define GLM_ENABLE_EXPERIMENTAL
#include "GltfAnimationClip.h"

GltfAnimationClip::GltfAnimationClip(std::string name) : mClipName(name) {}

void GltfAnimationClip::addChannel(std::shared_ptr<tinygltf::Model> model,
    tinygltf::Animation anim, tinygltf::AnimationChannel channel) {
    std::shared_ptr<GltfAnimationChannel> chan = std::make_shared<GltfAnimationChannel>();
    chan->loadChannelData(model, anim, channel);
    mAnimationChannels.push_back(chan);
}

void GltfAnimationClip::setAnimationFrame(std::vector<std::shared_ptr<GltfNode>> nodes,
    std::vector<bool> additiveMask, float time) {
    for (auto& channel : mAnimationChannels) {
        int targetNode = channel->getTargetNode();
        /* do not change if masked out */
        if (additiveMask.at(targetNode)) {
            switch (channel->getTargetPath()) {
            case ETargetPath::ROTATION:
                nodes.at(targetNode)->setRotation(channel->getRotation(time));
                break;
            case ETargetPath::TRANSLATION: { // <-- Äîáàâèëè ôèãóðíûå ñêîáêè
                glm::vec3 translation = channel->getTranslation(time);
                std::shared_ptr<GltfNode> tNode = nodes.at(targetNode);

                // ÕÀÊ ÄËß ÎÑÒÀÍÎÂÊÈ ÍÀ ÌÅÑÒÅ (In-Place)
                // Åñëè ýòî êîñòü òàçà Mixamo, ñáðàñûâàåì äâèæåíèå ïî îñÿì X è Z â íîëü
                if (tNode->getNodeName() == "mixamorig:Hips" || tNode->getNodeName() == "Hips") {
                    translation.x = 0.0f;
                    translation.z = 0.0f;
                }

                tNode->setTranslation(translation);
                break;
            } // <-- Çàêðûëè ôèãóðíûå ñêîáêè
            case ETargetPath::SCALE:
                nodes.at(targetNode)->setScale(channel->getScaling(time));
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
            case ETargetPath::TRANSLATION: { // <-- Äîáàâèëè ôèãóðíûå ñêîáêè
                glm::vec3 translation = channel->getTranslation(time);
                std::shared_ptr<GltfNode> tNode = nodes.at(targetNode);

                // ÕÀÊ ÄËß ÎÑÒÀÍÎÂÊÈ ÍÀ ÌÅÑÒÅ ÏÐÈ ÁËÅÍÄÈÍÃÅ
                if (tNode->getNodeName() == "mixamorig:Hips" || tNode->getNodeName() == "Hips") {
                    translation.x = 0.0f;
                    translation.z = 0.0f;
                }

                tNode->blendTranslation(translation, blendFactor);
                break;
            } // <-- Çàêðûëè ôèãóðíûå ñêîáêè
            case ETargetPath::SCALE:
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

