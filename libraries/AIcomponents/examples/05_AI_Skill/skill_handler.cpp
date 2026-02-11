/**
 * @file skill_handler.cpp
 * @brief Skill event handlers implementation
 */

#include "skill_handler.h"
#include "TuyaAI.h"
#include "File.h"
#include "Log.h"

#define TTS_BUFFER_SIZE     (128 * 1024)
#define TTS_AUDIO_PATH      "/sdcard/tts_output.mp3"
#define LED_PIN             1

static VFSFILE gSD(SDCARD);
static uint8_t *gTtsBuffer = NULL;
static uint32_t gTtsLen = 0;

void skillInit(void)
{
    gTtsBuffer = (uint8_t *)Malloc(TTS_BUFFER_SIZE);
    if (!gTtsBuffer) {
        PR_WARN("TTS buffer alloc failed");
    }
}

void skillCollectTts(uint8_t *data, uint32_t len)
{
    if (data && len > 0 && gTtsBuffer && gTtsLen + len <= TTS_BUFFER_SIZE) {
        memcpy(gTtsBuffer + gTtsLen, data, len);
        gTtsLen += len;
    }
}

void skillSaveTts(void)
{
    if (gTtsLen == 0 || !gTtsBuffer) return;
    
    TUYA_FILE fd = gSD.open(TTS_AUDIO_PATH, "wb");
    if (fd) {
        gSD.write((const char *)gTtsBuffer, gTtsLen, fd);
        gSD.close(fd);
        Serial.print("[TTS] Saved ");
        Serial.print(gTtsLen);
        Serial.println(" bytes");
    }
    gTtsLen = 0;
}

void skillHandleEvent(void *data)
{
    Serial.print("\n[EVT] AI_USER_EVT_SKILL");
    if (data != nullptr) {
        SkillData_t skill;
        if (OPRT_OK == TuyaAI.Skill.parse(data, skill)) {
            Serial.print(" code:"); Serial.print(skill.code);
            Serial.print(" type:"); Serial.print(skill.type);
            
            switch (skill.type) {
                case SKILL_TYPE_MUSIC:
                case SKILL_TYPE_STORY:
                    if (skill.music != nullptr) {
                        Serial.print("\n[Music] action:"); Serial.print(skill.music->action);
                        Serial.print("\n[Music] count:"); Serial.print(skill.music->src_cnt);
                        Serial.print("\n[Music] url:"); Serial.print(skill.music->src_array[0].url);
                        TuyaAI.Skill.dumpMusic(skill);
                        TuyaAI.Skill.freeMusic(skill);
                    }
                    break;
                    
                case SKILL_TYPE_PLAY_CONTROL:
                    Serial.print("\n[PlayControl] action:");
                    Serial.print(skill.playControl.action);
                    break;
                    
                case SKILL_TYPE_EMOTION:
                    Serial.print("\n[Emotion skill]");
                    break;
                    
                default:
                    break;
            }
        }
    }
}

void skillHandleEmotion(AI_AGENT_EMO_T *emo)
{
    if (!emo) return;
    
    Serial.print("[Emotion] ");
    Serial.print(emo->name ? emo->name : "unknown");
    
    if (emo->emoji) {
        char utf8[8];
        if (TuyaAI.Skill.unicodeToUtf8(emo->emoji, utf8, sizeof(utf8)) > 0) {
            Serial.print(" ");
            Serial.print(utf8);
        }
    }
    Serial.println();
}
