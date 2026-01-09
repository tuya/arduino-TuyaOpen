#ifndef __TUYA_AI_H_
#define __TUYA_AI_H_

#include "TuyaIoT.h"

#include "tuya_cloud_types.h"
#include "tal_api.h"

#include "inc/ai_audio.h"
#include "board_com_api.h"

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
    uint8_t getWorkMode();

    // Audio ctrl
    int setOpen(bool is_open);
    int setWakeup();
    int manualStartSingleTalk();
    int manualStopSingleTalk();
    AI_AUDIO_STATE_E getAudioState();

    // Player ctrl
    uint8_t isPlaying();
    int startPlaying(char *id);
    int stopPlaying();
    int playerDataWrite(char *id, uint8_t *data, uint32_t len, uint8_t is_eof);

    // Agent ctrl
    int textInput(uint8_t* data, size_t len);
    int audioInput(uint8_t* data, size_t len);
    int uploadStart(uint8_t enable_vad);
    int uploadStop();
    int uploadInterrupt(uint8_t enable_vad);

    // ASR ctrl
    int asrStart();
    int asrStop();
    int getAsrState();

    // Input ctrl
    int enableGetValidData(bool is_enable);
    int manualOpenGetValidData(bool is_open);
    int stopAsrAwake();
    int restartAsrAwakeTimer();
    uint32_t getInputData(uint8_t *buff, uint32_t len);
    uint32_t getInputDataSize();
    void discardInputData(uint32_t discard_size);

private:

};

#endif
