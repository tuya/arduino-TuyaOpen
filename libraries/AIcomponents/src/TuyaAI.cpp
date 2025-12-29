#include "TuyaAI.h"

extern "C" {
#include "tkl_output.h"
#include "tal_api.h"
#include "src/media/media_src_zh.h"
#include "lang_config.h"

}

typedef uint8_t APP_CHAT_MODE_E;
/*Press and hold button to start a single conversation.*/
#define APP_CHAT_MODE_KEY_PRESS_HOLD_SINGLE 0
/*Press the button once to start or stop the free conversation.*/
#define APP_CHAT_MODE_KEY_TRIG_VAD_FREE 1
/*Say the wake-up word to start a single conversation, similar to a smart speaker.
 *If no conversation is detected within 20 seconds, you need to say the wake-up word again*/
#define APP_CHAT_MODE_ASR_WAKEUP_SINGLE 2
/*Saying the wake-up word, you can have a free conversation.
 *If no conversation is detected within 20 seconds, you need to say the wake-up word again*/
#define APP_CHAT_MODE_ASR_WAKEUP_FREE 3

typedef struct {
    APP_CHAT_MODE_E mode;
    AI_AUDIO_WORK_MODE_E auido_mode;
    AI_AUDIO_ALERT_TYPE_E mode_alert;
    char *display_text;
    bool is_open;
} CHAT_WORK_MODE_INFO_T;

typedef struct {
    uint8_t is_enable;
    const CHAT_WORK_MODE_INFO_T *work;
} APP_CHAT_BOT_S;

const CHAT_WORK_MODE_INFO_T cAPP_WORK_HOLD = {
    .mode = APP_CHAT_MODE_KEY_PRESS_HOLD_SINGLE,
    .auido_mode = AI_AUDIO_MODE_MANUAL_SINGLE_TALK,
    .mode_alert = AI_AUDIO_ALERT_LONG_KEY_TALK,
    .display_text = HOLD_TALK,
    .is_open = true,
};

const CHAT_WORK_MODE_INFO_T cAPP_WORK_TRIG_VAD = {
    .mode = APP_CHAT_MODE_KEY_TRIG_VAD_FREE,
    .auido_mode = AI_AUDIO_WORK_VAD_FREE_TALK,
    .mode_alert = AI_AUDIO_ALERT_KEY_TALK,
    .display_text = TRIG_TALK,
    .is_open = false,
};

const CHAT_WORK_MODE_INFO_T cAPP_WORK_WAKEUP_SINGLE = {
    .mode = APP_CHAT_MODE_ASR_WAKEUP_SINGLE,
    .auido_mode = AI_AUDIO_WORK_ASR_WAKEUP_SINGLE_TALK,
    .mode_alert = AI_AUDIO_ALERT_WAKEUP_TALK,
    .display_text = WAKEUP_TALK,
    .is_open = true,
};

const CHAT_WORK_MODE_INFO_T cAPP_WORK_WAKEUP_FREE = {
    .mode = APP_CHAT_MODE_ASR_WAKEUP_FREE,
    .auido_mode = AI_AUDIO_WORK_ASR_WAKEUP_FREE_TALK,
    .mode_alert = AI_AUDIO_ALERT_FREE_TALK,
    .display_text = FREE_TALK,
    .is_open = true,
};


static APP_CHAT_BOT_S sg_chat_bot = {
    .is_enable = 0,
#if defined(ENABLE_CHAT_MODE_KEY_PRESS_HOLD_SINGEL) && (ENABLE_CHAT_MODE_KEY_PRESS_HOLD_SINGEL == 1)
    .work = &cAPP_WORK_HOLD,
#elif defined(ENABLE_CHAT_MODE_KEY_TRIG_VAD_FREE) && (ENABLE_CHAT_MODE_KEY_TRIG_VAD_FREE == 1)
    .work = &cAPP_WORK_TRIG_VAD,
#elif defined(ENABLE_CHAT_MODE_ASR_WAKEUP_SINGEL) && (ENABLE_CHAT_MODE_ASR_WAKEUP_SINGEL == 1)
    .work = &cAPP_WORK_WAKEUP_SINGLE,
#elif defined(ENABLE_CHAT_MODE_ASR_WAKEUP_FREE) && (ENABLE_CHAT_MODE_ASR_WAKEUP_FREE == 1)
    .work = &cAPP_WORK_WAKEUP_FREE,
#else
    .work = &cAPP_WORK_HOLD,  // Default fallback
#endif
};

static OPERATE_RET __app_chat_bot_enable(uint8_t enable)
{
    if (sg_chat_bot.is_enable == enable) {
        PR_DEBUG("chat bot enable is already %s", enable ? "enable" : "disable");
        return OPRT_OK;
    }

    PR_DEBUG("chat bot enable set %s", enable ? "enable" : "disable");

    ai_audio_set_open(enable);

    sg_chat_bot.is_enable = enable;

    return OPRT_OK;
}

TuyaAI::TuyaAI() {
}

TuyaAI::~TuyaAI() {
}

void TuyaAI::begin(AI_AUDIO_CONFIG_T ai_audio_cfg) {
    // Initialize AI components here
    // if (sg_chat_bot.work != NULL) {
    //     sg_chat_bot.work->auido_mode = ai_audio_cfg.work_mode;
    // }
    
    ai_audio_init(&ai_audio_cfg);

    if (sg_chat_bot.work != NULL) {
        __app_chat_bot_enable(sg_chat_bot.work->is_open);
    }
}

void TuyaAI::end() {
    // Clean up AI components here
    __app_chat_bot_enable(0);
}

// Volume control
int TuyaAI::setVolume(uint8_t volume) {
    return ai_audio_set_volume(volume);
}

uint8_t TuyaAI::getVolume() {
    return ai_audio_get_volume();
}

// Audio control
int TuyaAI::setOpen(bool is_open) {
    return ai_audio_set_open(is_open);
}

int TuyaAI::manualStartSingleTalk() {
    return ai_audio_manual_start_single_talk();
}

int TuyaAI::manualStopSingleTalk() {
    return ai_audio_manual_stop_single_talk();
}

int TuyaAI::setWakeup() {
    return ai_audio_set_wakeup();
}

AI_AUDIO_STATE_E TuyaAI::getState() {
    return ai_audio_get_state();
}

// Text input
int TuyaAI::textInput(uint8_t* text, size_t length) {
    return ai_text_agent_upload(text, length);
}