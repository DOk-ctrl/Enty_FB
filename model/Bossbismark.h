#pragma once
#include "Enemy.h"
#include "audio.h"
// ============================================================
// Ѕосс є2 Ч быстрый боец.
//
//  липы (пор€док из лога загрузки GltfModel::getAnimations):
//   0 = Action.001     (не используем Ч резерв)
//   1 = deafault kick
//   2 = Dying
//   3 = Flying Kick
//   4 = Idle
//   5 = MMA Kick
//   6 = Reaction
//   7 = Walk
//
// ¬ отличие от Conway у этого босса полный набор: есть Walk и Reaction,
// поэтому canBeStaggered() == true (стан работает как у обычного врага),
// а блока нет вовсе (canBlock() == false).
// ‘ракции окон урона Ч начальные оценки, подберите точнее по факту клипов.
// ============================================================
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
    std::string getBossName() const override { return "BISMARK"; } // подставь реальное им€
};