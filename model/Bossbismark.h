#pragma once
#include "Enemy.h"
#include "audio.h"

class BossBismark : public Enemy {
public:
    BossBismark(std::shared_ptr<GltfInstance> model, btRigidBody* body);

    int getAnimIndex(EnemyAnimSlot slot) const override;
    bool shouldResetPosition() const override;
    std::vector<EnemyAnimSlot> getAvailableAttackSlots() const override;
    std::vector<HitWindow> getHitWindows(EnemyAnimSlot attackSlot) const override;
    bool canBeStaggered() const override {
        return mStunCount < 3;
    }
    float getAttackRange() const override { return 1.8f; }
    bool  canBlock() const override { return false; }

    bool  isBismark() const override { return true; }
    void  playmusic() override { gAudio.playMusic("assets/audio/bismusic.mp3"); }
    void onAttackPhysicsUpdate(float attackTimeAccumulator, float deltaTime, EnemyAnimSlot currentAttackAnim) override;
    bool isBoss() const override { return true; }
    std::string getBossName() const override { return "BISMARK"; } };