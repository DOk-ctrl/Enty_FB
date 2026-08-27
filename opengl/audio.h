#pragma once
#include "miniaudio.h"  

class AudioEngine {
public:
    bool init();
    void shutdown();

    void playSound(const char* filePath);

    void playMusic(const char* filePath, bool loop = true);
    void stopMusic();

private:
    ma_engine engine;
    ma_sound  musicSound;
    bool      musicLoaded = false;
};
extern AudioEngine gAudio;
