#pragma once
#include <vector>

// ”ниверсальный "слот" анимации Ч каждый враг (симбиот, боссы) сам решает,
// какой индекс клипа (из своего GltfModel::getAnimations) соответствует слоту.
// -1 из getAnimIndex() означает "этот враг так не умеет" (напр. Block у Symbiote,
// HitReaction у Conway, Move у Conway).
enum class EnemyAnimSlot {
    Idle,
    Move,         // ходьба/бег в погоне
    Attack1,
    Attack2,
    Attack3,
    Block,
    HitReaction,
    Death
};

// ќкно урона внутри анимации атаки, в дол€х от еЄ длительности (0..1).
// Ќесколько окон в одном списке = комбо с несколькими ударами за клип.
struct HitWindow {
    float startFrac = 0.55f;
    float endFrac = 0.65f;
    bool  consumed = false;
};