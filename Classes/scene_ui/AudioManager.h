// Copyright 2025 The Black Myth Wukong Authors. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__

#include <string>

#include "audio/include/AudioEngine.h"
#include "cocos2d.h"

// AudioManager manages background music and sound effects for the game.
// It provides a singleton interface for playing, stopping, and controlling volume.
class AudioManager {
 public:
  // Returns the singleton instance of AudioManager.
  static AudioManager* getInstance();

  // Plays background music from the given file path.
  // @param fileName The path to the music file.
  // @param loop Whether to loop the music (default is true).
  void playBGM(const std::string& fileName, bool loop = true);

  // Stops the currently playing background music.
  void stopBGM();

  // Plays a sound effect from the given file path.
  // @param fileName The path to the sound effect file.
  // @param loop Whether to loop the sound effect (default is false).
  // @return The unique ID of the playing audio.
  int playEffect(const std::string& fileName, bool loop = false);

  // Stops a specific sound effect by its ID.
  // @param audioID The ID of the sound effect to stop.
  void stopEffect(int audioID);

  // Stops all sounds (BGM and effects).
  void stopAll();

  // Sets the volume for background music.
  // @param volume The volume level (0.0 to 1.0).
  void setBGMVolume(float volume);

  // Sets the volume for all subsequent sound effects.
  // @param volume The volume level (0.0 to 1.0).
  void setEffectVolume(float volume);

 private:
  AudioManager();
  ~AudioManager();

  static AudioManager* _instance;
  int _bgmID;           // Current background music ID.
  float _bgmVolume;     // Background music volume level.
  float _effectVolume;  // Sound effects volume level.
};

#endif  // __AUDIO_MANAGER_H__
