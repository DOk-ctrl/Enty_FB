// AudioEngine.h
#pragma once
#include "miniaudio.h"  // без MINIAUDIO_IMPLEMENTATION здесь!

class AudioEngine {
public:
    bool init();
    void shutdown();

    // старый метод Ч одноразовый звук (эффекты, клики и т.п.)
    void playSound(const char* filePath);

    // новые методы Ч управл€ема€ музыка (можно зациклить/остановить)
    void playMusic(const char* filePath, bool loop = true);
    void stopMusic();

private:
    ma_engine engine;
    ma_sound  musicSound;
    bool      musicLoaded = false;
};
// √лобальный доступ из любого .cpp файла проекта
extern AudioEngine gAudio;
