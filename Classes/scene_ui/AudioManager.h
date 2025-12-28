// Copyright 2025 The Black Myth Wukong Authors. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__

#include <string>

#include "audio/include/AudioEngine.h"
#include "cocos2d.h"

// AudioManager 管理游戏的背景音乐和音效。
// 它提供了一个单例接口，用于播放、停止和控制音量。
class AudioManager {
 public:
  // 获取 AudioManager 的单例实例。
  static AudioManager* getInstance();

  // 播放指定路径的背景音乐。
  // @param fileName 音乐文件的路径。
  // @param loop 是否循环播放（默认为 true）。
  void playBGM(const std::string& fileName, bool loop = true);

  // 停止当前正在播放的背景音乐。
  void stopBGM();

  // 播放指定路径的音效。
  // @param fileName 音效文件的路径。
  // @param loop 是否循环播放（默认为 false）。
  // @return 播放中的音频唯一 ID。
  int playEffect(const std::string& fileName, bool loop = false);

  // 通过 ID 停止特定的音效。
  // @param audioID 要停止的音效 ID。
  void stopEffect(int audioID);

  // 停止所有声音（背景音乐和音效）。
  void stopAll();

  // 设置背景音乐的音量。
  // @param volume 音量级别（0.0 到 1.0）。
  void setBGMVolume(float volume);

  // 设置后续所有音效的音量。
  // @param volume 音量级别（0.0 到 1.0）。
  void setEffectVolume(float volume);

 private:
  AudioManager();
  ~AudioManager();

  static AudioManager* _instance;
  int _bgmID;           // 当前背景音乐 ID。
  float _bgmVolume;     // 背景音乐音量级别。
  float _effectVolume;  // 音效音量级别。
};

#endif  // __AUDIO_MANAGER_H__
