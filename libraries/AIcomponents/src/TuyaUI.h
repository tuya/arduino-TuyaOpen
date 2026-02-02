/**
 * @file TuyaUI.h
 * @author Tuya Inc.
 * @brief TuyaUIClass declaration and UI-related types
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */
#ifndef __TUYA_UI_H_
#define __TUYA_UI_H_

#include "tuya_cloud_types.h"
#include "ai_ui_manage.h"
#include "ai_manage_mode.h"


#if defined(ENABLE_LIBLVGL) && (ENABLE_LIBLVGL == 1)
#include "lvgl.h"
#include "font_awesome_symbols.h"
#endif

/***********************************************************
************************macro define************************
***********************************************************/
// UI type selection macros
#define BOT_UI_USER         0       /**< User custom UI (appDisplay.cpp) */
#define BOT_UI_WECHAT       1       /**< WeChat UI */
// #define BOT_UI_CHATBOT      2       /**< Chatbot UI */
// #define BOT_UI_OLED         3       /**< OLED display UI */

/***********************************************************
***********************typedef define***********************
***********************************************************/

/**
 * @brief UI display callback function type (for custom UI)
 * @param type Display message type (AI_UI_DISP_TYPE_E from ai_ui_manage.h)
 * @param data Message data
 * @param len  Message length
 */
typedef void (*UIDisplayCallback_t)(AI_UI_DISP_TYPE_E type, uint8_t *data, int len);

/**
 * @brief UI Configuration Structure
 */
typedef struct {
    int                 uiType;         /**< UI type (BOT_UI_USER, BOT_UI_DEFAULT, etc.) */
    UIDisplayCallback_t displayCb;      /**< Custom display callback (for BOT_UI_USER) */
} UIConfig_t;

typedef struct {
    char  emo_name[32];
    char *emo_icon;
} UI_EMOJI_LIST_t;
/***********************************************************
***********************class definition*********************
***********************************************************/

/**
 * @class TuyaUIClass
 * @brief Nested class for UI display management
 * 
 * Provides interface for managing display output including
 * messages, emotions, status, and notifications.
 */
class TuyaUIClass {
public:
    TuyaUIClass();
    ~TuyaUIClass();
    
    /**
     * @brief Initialize UI with type selection
     * @param uiType UI type (BOT_UI_USER=0 for custom, BOT_UI_DEFAULT=1 for built-in)
     * @return OPRT_OK on success, error code on failure
     * @note When uiType=0, also call appDisplayInit() after this
     */
    OPERATE_RET begin(int uiType = BOT_UI_WECHAT);
    
    /**
     * @brief Initialize UI with configuration
     * @param config UI configuration structure
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET begin(UIConfig_t &config);
    
    /**
     * @brief Deinitialize UI
     */
    void end();
    
    /**
     * @brief Check if UI is initialized
     * @return true if initialized
     */
    bool isInitialized();
    
    /**
     * @brief Get current UI type
     * @return UI type value
     */
    int getType();
    
    /**
     * @brief Display message
     * @param type Display type (AI_UI_DISP_TYPE_E from ai_ui_manage.h)
     * @param data Message data
     * @param len  Message length
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET displayMessage(AI_UI_DISP_TYPE_E type, uint8_t *data, int len);
    
    /**
     * @brief Display text message
     * @param text Text to display
     * @param isUserMessage true for user message, false for AI message
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET displayText(const char *text, bool isUserMessage = false);
    
    /**
     * @brief Display emotion
     * @param emotion Emotion name (e.g., "HAPPY", "SAD")
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET displayEmotion(const char *emotion);
    
    /**
     * @brief Display system notification
     * @param notification Notification text
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET displayNotification(const char *notification);
    
    /**
     * @brief Display WiFi status
     * @param status WiFi status (AI_UI_WIFI_STATUS_E from ai_ui_manage.h)
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET displayWifiStatus(AI_UI_WIFI_STATUS_E status);
    
    /**
     * @brief Display chat mode
     * @param mode Chat mode (AI_CHAT_MODE_E from ai_manage_mode.h)
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET displayChatMode(AI_CHAT_MODE_E mode);

    /**
     * @brief Enable LVGL update
     * @return void
     */
    void enableUpdateLVGL();

    /**
     * @brief Disable LVGL update
     * @return void
     */
    void disableUpdateLVGL();
    
    /**
     * @brief Set custom display callback
     * @param callback Display callback function
     */
    void setDisplayCallback(UIDisplayCallback_t callback);
    
#if defined(ENABLE_LIBLVGL) && (ENABLE_LIBLVGL == 1)
    /***********************************************************
    ************************Font Resource APIs*******************
    ***********************************************************/
    
    /**
     * @brief Get text font for Chinese/English display
     * @return Pointer to lv_font_t for text rendering
     */
    lv_font_t *getTextFont();
    
    /**
     * @brief Get icon font (FontAwesome)
     * @return Pointer to lv_font_t for icon rendering
     */
    lv_font_t *getIconFont();
    
    /**
     * @brief Get emoji font
     * @return Pointer to lv_font_t for emoji rendering
     */
    lv_font_t *getEmoFont();
    
    /**
     * @brief Get emoji list for name-to-icon mapping
     * @return Pointer to UI_EMOJI_LIST_t array
     */
    UI_EMOJI_LIST_t *getEmoList();
    
    /**
     * @brief Get WiFi icon by status
     * @param status WiFi status (AI_UI_WIFI_STATUS_E from ai_ui_manage.h)
     * @return Icon string for WiFi status
     */
    char *getWifiIcon(AI_UI_WIFI_STATUS_E status);
#endif

private:
    bool _initialized;
    int  _uiType;
    UIDisplayCallback_t _displayCallback;
};

#endif /* __TUYA_UI_H_ */
