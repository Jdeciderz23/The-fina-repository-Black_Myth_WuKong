#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__

#include "cocos2d.h"
#include "audio/include/AudioEngine.h"
#include <string>

/**
 * @class AudioManager
 * @brief 游戏音效管理类，负责背景音乐和音效的播放与控制
 */
class AudioManager {
public:
    // 获取单例实例
    static AudioManager* getInstance();

    /**
     * @brief 播放背景音乐
     * @param fileName 音乐文件路径
     * @param loop 是否循环播放，默认为 true
     */
    void playBGM(const std::string& fileName, bool loop = true);

    /**
     * @brief 停止当前背景音乐
     */
    void stopBGM();

    /**
     * @brief 播放音效
     * @param fileName 音效文件路径
     * @param loop 是否循环播放，默认为 false
     * @return 音效的 ID
     */
    int playEffect(const std::string& fileName, bool loop = false);

    /**
     * @brief 停止指定 ID 的音效
     * @param audioID 音效 ID
     */
    void stopEffect(int audioID);

    /**
     * @brief 停止所有声音
     */
    void stopAll();

    /**
     * @brief 设置背景音乐音量
     * @param volume 音量 (0.0 - 1.0)
     */
    void setBGMVolume(float volume);

    /**
     * @brief 设置所有音效音量
     * @param volume 音量 (0.0 - 1.0)
     */
    void setEffectVolume(float volume);

private:
    AudioManager();
    ~AudioManager();

    static AudioManager* _instance;
    int _bgmID;           // 当前背景音乐的 ID
    float _bgmVolume;     // 背景音乐音量
    float _effectVolume;  // 音效音量
};

#endif // __AUDIO_MANAGER_H__
