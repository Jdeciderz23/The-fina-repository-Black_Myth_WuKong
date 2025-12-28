// Copyright 2025 The Black Myth Wukong Authors. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AudioManager.h"

#include <algorithm>

USING_NS_CC;

AudioManager* AudioManager::_instance = nullptr;

AudioManager* AudioManager::getInstance() {
  if (!_instance) {
    _instance = new AudioManager();
  }
  return _instance;
}

AudioManager::AudioManager()
    : _bgmID(AudioEngine::INVALID_AUDIO_ID),
      _bgmVolume(1.0f),
      _effectVolume(1.0f) {}

AudioManager::~AudioManager() {
  AudioEngine::end();
}

void AudioManager::playBGM(const std::string& fileName, bool loop) {
  // If there's already background music playing, stop it first.
  if (_bgmID != AudioEngine::INVALID_AUDIO_ID) {
    AudioEngine::stop(_bgmID);
  }

  // Play the new background music.
  _bgmID = AudioEngine::play2d(fileName, loop, _bgmVolume);

  // If playback fails, reset the ID.
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
  // Note: AudioEngine does not provide a direct interface to set the volume
  // of all currently playing sound effects. Usually, you need to manage a
  // list of sound effect IDs yourself or only affect subsequent playback.
}
