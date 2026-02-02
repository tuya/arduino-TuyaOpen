/**
 * @file TuyaSkill.h
 * @brief TuyaSkill C++ wrapper for AI skill parsing
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#ifndef __TUYA_SKILL_H_
#define __TUYA_SKILL_H_

#include "tuya_cloud_types.h"
#include "skill_emotion.h"
#include "skill_music_story.h"
#include "ai_audio_player.h"


#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
// Skill codes
#define SKILL_CODE_EMO          "emo"
#define SKILL_CODE_LLM_EMO      "llm_emo"
#define SKILL_CODE_MUSIC        "music"
#define SKILL_CODE_STORY        "story"
#define SKILL_CODE_PLAY_CONTROL "PlayControl"

// Emotion macros: use EMOJI_* from skill_emotion.h

/***********************************************************
***********************typedef define***********************
***********************************************************/

/** @brief Skill type enumeration */
typedef enum {
    SKILL_TYPE_UNKNOWN = 0,
    SKILL_TYPE_EMOTION,
    SKILL_TYPE_MUSIC,
    SKILL_TYPE_STORY,
    SKILL_TYPE_PLAY_CONTROL,
    SKILL_TYPE_CLOUD_EVENT,
} SkillType_t;

/** @brief Play control data */
typedef struct {
    char action[16];
} SkillPlayControl_t;

/** @brief Cloud event data */
typedef struct {
    char action[24];
    char ttsUrl[256];
    char format[16];
} SkillCloudEvent_t;

/** @brief Unified skill data structure */
typedef struct {
    SkillType_t type;
    char code[32];
    union {
        AI_AGENT_EMO_T *emotion;         /**< Emotion data (from skill_emotion.h) */
        AI_AUDIO_MUSIC_T *music;         /**< Music/Story data (from ai_audio_player.h) */
        SkillPlayControl_t playControl;
        SkillCloudEvent_t cloudEvent;
    };
} SkillData_t;

#ifdef __cplusplus
}
#endif

/***********************************************************
***********************class definition*********************
***********************************************************/

#ifdef __cplusplus

/**
 * @class TuyaSkillClass
 * @brief AI skill callback data parser
 */
class TuyaSkillClass {
public:
    TuyaSkillClass();
    ~TuyaSkillClass();

    /**
     * @brief Parse skill data from AI_USER_EVT_SKILL callback
     * @param data cJSON pointer from callback
     * @param skill Output skill data
     * @return OPRT_OK on success
     * @note Call freeMusic() after using MUSIC/STORY type
     */
    OPERATE_RET parse(void *data, SkillData_t &skill);
    
    /**
     * @brief Free music data allocated by parse()
     * @param skill Skill data with music pointer
     */
    void freeMusic(SkillData_t &skill);
    
    /**
     * @brief Dump music data for debugging
     */
    void dumpMusic(const SkillData_t &skill);
    
    /**
     * @brief Convert Unicode to UTF-8
     * @return Bytes written, -1 on error
     */
    int unicodeToUtf8(const char *unicode, char *utf8, size_t bufSize);
    
    /**
     * @brief Get emotion name from unicode
     * @return Name or NULL
     */
    const char* getEmotionName(const char *emoji);
    
    /**
     * @brief Get unicode from emotion name
     * @return Unicode or NULL
     */
    const char* getEmotionEmoji(const char *name);
    
    // Type helpers
    static bool isEmotion(const SkillData_t &skill) { return skill.type == SKILL_TYPE_EMOTION; }
    static bool isMusic(const SkillData_t &skill) { return skill.type == SKILL_TYPE_MUSIC; }
    static bool isStory(const SkillData_t &skill) { return skill.type == SKILL_TYPE_STORY; }
    static bool isPlayControl(const SkillData_t &skill) { return skill.type == SKILL_TYPE_PLAY_CONTROL; }

private:
    OPERATE_RET _parseEmotionSkill(void *json, SkillData_t &skill);
    OPERATE_RET _parseMusicSkill(void *json, SkillData_t &skill);
    OPERATE_RET _parsePlayControlSkill(void *json, SkillData_t &skill);
};

#endif // __cplusplus

#endif /* __TUYA_SKILL_H_ */
