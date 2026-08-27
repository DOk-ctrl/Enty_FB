#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "audio.h"

AudioEngine gAudio; 
bool AudioEngine::init() {
    ma_result result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        return false;
    }
    return true;
}

void AudioEngine::playSound(const char* filePath) {
    ma_engine_play_sound(&engine, filePath, NULL);
}

void AudioEngine::playMusic(const char* filePath, bool loop) {
        stopMusic();

    ma_result result = ma_sound_init_from_file(
        &engine, filePath,
        MA_SOUND_FLAG_STREAM,         NULL, NULL,
        &musicSound
    );
    if (result != MA_SUCCESS) {
        return;
    }

    ma_sound_set_looping(&musicSound, loop ? MA_TRUE : MA_FALSE);
    ma_sound_start(&musicSound);
    musicLoaded = true;
}

void AudioEngine::stopMusic() {
    if (musicLoaded) {
        ma_sound_stop(&musicSound);
        ma_sound_uninit(&musicSound);
        musicLoaded = false;
    }
}

void AudioEngine::shutdown() {
    stopMusic();
    ma_engine_uninit(&engine);
}