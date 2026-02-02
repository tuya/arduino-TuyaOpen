/**
 * @file TuyaSkill.cpp
 * @brief TuyaSkill class implementation
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#include "TuyaSkill.h"
#include "cJSON.h"
#include "tal_api.h"

#include "skill_emotion.h"
#include "skill_music_story.h"

/***********************************************************
***********************class implementation*****************
***********************************************************/

TuyaSkillClass::TuyaSkillClass()
{
}

TuyaSkillClass::~TuyaSkillClass()
{
}

OPERATE_RET TuyaSkillClass::parse(void *data, SkillData_t &skill)
{
    if (data == nullptr) {
        return OPRT_INVALID_PARM;
    }
    
    cJSON *root = (cJSON *)data;
    cJSON *node = cJSON_GetObjectItem(root, "code");
    const char *code = cJSON_GetStringValue(node);
    
    if (code == nullptr) {
        return OPRT_CJSON_GET_ERR;
    }
    
    memset(&skill, 0, sizeof(SkillData_t));
    strncpy(skill.code, code, sizeof(skill.code) - 1);
    
    if (strcmp(code, SKILL_CODE_EMO) == 0 || strcmp(code, SKILL_CODE_LLM_EMO) == 0) {
        skill.type = SKILL_TYPE_EMOTION;
        return _parseEmotionSkill(root, skill);
    }
    else if (strcmp(code, SKILL_CODE_MUSIC) == 0) {
        skill.type = SKILL_TYPE_MUSIC;
        return _parseMusicSkill(root, skill);
    }
    else if (strcmp(code, SKILL_CODE_STORY) == 0) {
        skill.type = SKILL_TYPE_STORY;
        return _parseMusicSkill(root, skill);
    }
    else if (strcmp(code, SKILL_CODE_PLAY_CONTROL) == 0) {
        skill.type = SKILL_TYPE_PLAY_CONTROL;
        return _parsePlayControlSkill(root, skill);
    }
    else {
        skill.type = SKILL_TYPE_UNKNOWN;
        return OPRT_OK;
    }
}

void TuyaSkillClass::freeMusic(SkillData_t &skill)
{
    if ((skill.type == SKILL_TYPE_MUSIC || skill.type == SKILL_TYPE_STORY) && 
        skill.music != nullptr) {
        ai_skill_parse_music_free(skill.music);
        skill.music = nullptr;
    }
}

void TuyaSkillClass::dumpMusic(const SkillData_t &skill)
{
    if ((skill.type == SKILL_TYPE_MUSIC || skill.type == SKILL_TYPE_STORY) && 
        skill.music != nullptr) {
        ai_skill_parse_music_dump(skill.music);
    }
}

int TuyaSkillClass::unicodeToUtf8(const char *unicode, char *utf8, size_t bufSize)
{
    return ai_emoji_unicode_to_utf8(unicode, utf8, bufSize);
}

const char* TuyaSkillClass::getEmotionName(const char *emoji)
{
    return ai_agent_emoji_get_name(emoji);
}

const char* TuyaSkillClass::getEmotionEmoji(const char *name)
{
    return ai_agent_emoji_get_by_name(name);
}

/***********************************************************
***********************private methods**********************
***********************************************************/

OPERATE_RET TuyaSkillClass::_parseEmotionSkill(void *json, SkillData_t &skill)
{
    cJSON *root = (cJSON *)json;
    cJSON *skillContent = cJSON_GetObjectItem(root, "skillContent");
    
    if (skillContent == nullptr) {
        return OPRT_CJSON_GET_ERR;
    }
    
    cJSON *emotionArr = cJSON_GetObjectItem(skillContent, "emotion");
    if (emotionArr == nullptr || cJSON_GetArraySize(emotionArr) == 0) {
        return OPRT_CJSON_GET_ERR;
    }
    
    skill.emotion = nullptr;
    return OPRT_OK;
}

OPERATE_RET TuyaSkillClass::_parseMusicSkill(void *json, SkillData_t &skill)
{
    cJSON *root = (cJSON *)json;
    OPERATE_RET ret = ai_skill_parse_music(root, &skill.music);
    if (ret != OPRT_OK) {
        skill.music = nullptr;
        return ret;
    }
    return (skill.music != nullptr) ? OPRT_OK : OPRT_MALLOC_FAILED;
}

OPERATE_RET TuyaSkillClass::_parsePlayControlSkill(void *json, SkillData_t &skill)
{
    cJSON *root = (cJSON *)json;
    cJSON *skillGeneral = cJSON_GetObjectItem(root, "general");
    cJSON *skillCustom = cJSON_GetObjectItem(root, "custom");
    cJSON *action = nullptr;
    
    if (skillCustom) {
        action = cJSON_GetObjectItem(skillCustom, "action");
    }
    if (action == nullptr && skillGeneral) {
        action = cJSON_GetObjectItem(skillGeneral, "action");
    }
    
    if (action && cJSON_IsString(action)) {
        strncpy(skill.playControl.action, action->valuestring, sizeof(skill.playControl.action) - 1);
    }
    
    return OPRT_OK;
}
