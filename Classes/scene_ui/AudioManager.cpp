#include "AudioManager.h"

USING_NS_CC;

AudioManager* AudioManager::_instance = nullptr;

AudioManager* AudioManager::getInstance() {
    if (!_instance) {
        _instance = new AudioManager();
    }
    return _instance;
}

AudioManager::AudioManager() 
    : _bgmID(AudioEngine::INVALID_AUDIO_ID)
    , _bgmVolume(1.0f)
    , _effectVolume(1.0f) {
}

AudioManager::~AudioManager() {
    AudioEngine::end();
}

void AudioManager::playBGM(const std::string& fileName, bool loop) {
    // 如果已经有背景音乐在播放，先停止它
    if (_bgmID != AudioEngine::INVALID_AUDIO_ID) {
        AudioEngine::stop(_bgmID);
    }

    // 播放新的背景音乐
    _bgmID = AudioEngine::play2d(fileName, loop, _bgmVolume);
    
    // 如果播放失败，重置 ID
    if (_bgmID == AudioEngine::INVALID_AUDIO_ID) {
        CCLOG("AudioManager: Failed to play BGM %s", fileName.c_str());
    }
}

void AudioManager::stopBGM() {
    if (_bgmID != AudioEngine::INVALID_AUDIO_ID) {
        AudioEngine::stop(_bgmID);
        _bgmID = AudioEngine::INVALID_AUDIO_ID;
    }
}

int AudioManager::playEffect(const std::string& fileName, bool loop) {
    return AudioEngine::play2d(fileName, loop, _effectVolume);
}

void AudioManager::stopEffect(int audioID) {
    AudioEngine::stop(audioID);
}

void AudioManager::stopAll() {
    AudioEngine::stopAll();
    _bgmID = AudioEngine::INVALID_AUDIO_ID;
}

void AudioManager::setBGMVolume(float volume) {
    _bgmVolume = std::max(0.0f, std::min(1.0f, volume));
    if (_bgmID != AudioEngine::INVALID_AUDIO_ID) {
        AudioEngine::setVolume(_bgmID, _bgmVolume);
    }
}

void AudioManager::setEffectVolume(float volume) {
    _effectVolume = std::max(0.0f, std::min(1.0f, volume));
    // 注意：AudioEngine 并没有直接设置所有正在播放音效音量的接口
    // 通常需要自己管理音效 ID 列表或者只影响后续播放的音效
}
