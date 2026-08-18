#pragma once
#include "Enemy.h"

// ============================================================
//
//
//  - Нет клипа реакции на урон -> canBeStaggered() == false: босс "проглатывает"
//    удары без прерывания (эффект poise, характерный для тяжёлых боссов).
//  - Фракции окон урона (getHitWindows) — начальные оценки "на глаз", подбирайте
//    точнее по факту просмотра реальных клипов в движке.
// ============================================================
class BossConway : public Enemy {
public:
    BossConway(std::shared_ptr<GltfInstance> model, btRigidBody* body);

    int getAnimIndex(EnemyAnimSlot slot) const override;
    std::vector<EnemyAnimSlot> getAvailableAttackSlots() const override;
    std::vector<HitWindow> getHitWindows(EnemyAnimSlot attackSlot) const override;

    float getAttackRange() const override { return 2.2f; }
    bool  canBlock() const override { return true; }
    bool  canBeStaggered() const override { return false; }
    void onAttackPhysicsUpdate(float attackTimeAccumulator, float deltaTime, EnemyAnimSlot currentAttackAnim) override;
    bool isBoss() const override { return true; }
    std::string getBossName() const override { return "CONWAY"; } // подставь реальное имя
};