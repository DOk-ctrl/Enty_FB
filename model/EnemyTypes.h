#pragma once
#include <vector>

enum class EnemyAnimSlot {
    Idle,
    Move,             Attack1,
    Attack2,
    Attack3,
    Block,
    HitReaction,
    Death
};

struct HitWindow {
    float startFrac = 0.55f;
    float endFrac = 0.65f;
    bool  consumed = false;
};