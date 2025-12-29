#ifndef __TUYA_AI_H_
#define __TUYA_AI_H_

#include "TuyaIoT.h"

extern "C"  {   
#include "inc/ai_audio.h"
#include "board_com_api.h"
}
typedef enum {
    AI_AUDIO_ALERT_NORMAL = 0,
    AI_AUDIO_ALERT_POWER_ON,
    AI_AUDIO_ALERT_NOT_ACTIVE,
    AI_AUDIO_ALERT_NETWORK_CFG,
    AI_AUDIO_ALERT_NETWORK_CONNECTED,
    AI_AUDIO_ALERT_NETWORK_FAIL,
    AI_AUDIO_ALERT_NETWORK_DISCONNECT,
    AI_AUDIO_ALERT_BATTERY_LOW,
    AI_AUDIO_ALERT_PLEASE_AGAIN,
    AI_AUDIO_ALERT_WAKEUP,
    AI_AUDIO_ALERT_LONG_KEY_TALK,
    AI_AUDIO_ALERT_KEY_TALK,
    AI_AUDIO_ALERT_WAKEUP_TALK,
    AI_AUDIO_ALERT_FREE_TALK,
} AI_AUDIO_ALERT_TYPE_E;

class TuyaAI {

public:

    TuyaAI();
    ~TuyaAI();

    // Initialize AI audio system
    void begin(AI_AUDIO_CONFIG_T ai_audio_cfg);
    void end();

    // Volume control
    int setVolume(uint8_t volume);
    uint8_t getVolume();

    // Audio control
    int setOpen(bool is_open);
    int manualStartSingleTalk();
    int manualStopSingleTalk();
    int setWakeup();
    AI_AUDIO_STATE_E getState();

    // Text input (for text-only mode)
    int textInput(uint8_t* text, size_t length);

private:

};

#endif
