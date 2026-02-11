/**
 * @file appDisplay.cpp
 * @brief Minimalist UI for AI ChatBot (Pure LVGL)
 *
 * A clean, minimalist UI design featuring:
 * - Compact header with mode indicator and WiFi icon
 * - Center status display with animated state
 * - Scrollable chat area with user/AI messages
 *
 * Layout (320x480 screen):
 * ┌──────────────────────────┐
 * │ [Mode]          [WiFi]  │ <- Header (32px)
 * ├──────────────────────────┤
 * │      [ Status ]          │ <- Status (40px)
 * ├──────────────────────────┤
 * │ ┌────────────────────┐   │
 * │ │ User message...    │◀──│ <- User bubble (right)
 * │ └────────────────────┘   │
 * │   ┌────────────────────┐ │
 * │ ──▶ AI response...     │ │ <- AI bubble (left)
 * │   └────────────────────┘ │
 * └──────────────────────────┘
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#include "appDisplay.h"
#include "Log.h"
#include "lang_config.h"
#include "lv_vendor.h"

/***********************************************************
************************macro define************************
***********************************************************/
// Layout dimensions (optimized for 320x480)
#define HEADER_H            30
#define STATUS_H            36
#define PADDING             6
#define BUBBLE_RADIUS       12
#define BUBBLE_MAX_W        100  // percentage

// Modern Dark Theme Colors
#define CLR_BG_DARK         0x1A1A2E    // Dark navy background
#define CLR_HEADER_BG       0x16213E    // Darker header
#define CLR_STATUS_BG       0x0F3460    // Status area blue
#define CLR_USER_BG         0x4ECCA3    // Mint green for user
#define CLR_AI_BG           0x393E46    // Dark gray for AI
#define CLR_TEXT_PRIMARY    0xEEEEEE    // Light text
#define CLR_TEXT_SECONDARY  0xA0A0A0    // Muted text
#define CLR_ACCENT          0x00ADB5    // Cyan accent

/***********************************************************
***********************variable define**********************
***********************************************************/
static bool gReady = false;

#if defined(UI_TYPE) && (UI_TYPE == BOT_UI_USER)
// UI elements
static lv_obj_t *sg_screen      = NULL;
static lv_obj_t *sg_header      = NULL;
static lv_obj_t *sg_lblMode     = NULL;
static lv_obj_t *sg_lblWifi     = NULL;
static lv_obj_t *sg_statusBox   = NULL;
static lv_obj_t *sg_lblStatus   = NULL;
static lv_obj_t *sg_chatArea    = NULL;
static lv_obj_t *sg_userBubble  = NULL;
static lv_obj_t *sg_lblUser     = NULL;
static lv_obj_t *sg_aiBubble    = NULL;
static lv_obj_t *sg_lblAI       = NULL;

// Stream buffer
static char sg_streamBuf[512]  = {0};
static int  sg_streamLen        = 0;

static void _createUI(void);
#endif

/***********************************************************
***********************public functions*********************
***********************************************************/

/**
 * @brief Initialize the display
 */
OPERATE_RET appDisplayInit(uint8_t uiType)
{
    if (OPRT_OK != TuyaAI.UI.begin(uiType)) {
        PR_ERR("TuyaAI.UI initialization failed");
    }
#if defined(UI_TYPE) && (UI_TYPE == BOT_UI_USER)
    _createUI();
#endif
    gReady = true;
    PR_NOTICE("UI: [%d] display initialized successfully", uiType);
    return OPRT_OK;
}

bool appDisplayIsReady(void) { return gReady; }

/**
 * @brief Clear all display content
 */
void appDisplayClear(void)
{
#if defined(UI_TYPE) && (UI_TYPE == BOT_UI_USER)
    if (sg_lblUser) lv_label_set_text(sg_lblUser, "");
    if (sg_lblAI) lv_label_set_text(sg_lblAI, "");
    if (sg_userBubble) lv_obj_add_flag(sg_userBubble, LV_OBJ_FLAG_HIDDEN);
    if (sg_aiBubble) lv_obj_add_flag(sg_aiBubble, LV_OBJ_FLAG_HIDDEN);
    sg_streamBuf[0] = '\0';
    sg_streamLen = 0;
#endif
}

/**
 * @brief Display user's spoken text
 * @param text The text to display
 */
void appDisplaySetUserText(const char *text)
{
    if (!gReady || !text) return;
#if defined(UI_TYPE) && (UI_TYPE == BOT_UI_USER)
    if (strlen(text) > 0) {
        lv_label_set_text(sg_lblUser, text);
        lv_obj_clear_flag(sg_userBubble, LV_OBJ_FLAG_HIDDEN);
        // Auto scroll to bottom
        lv_obj_scroll_to_y(sg_chatArea, LV_COORD_MAX, LV_ANIM_ON);
    }
#else
    // TuyaAI.UI.displayMessage(UI_DISP_USER_MSG, (uint8_t *)text, strlen(text));
    PR_NOTICE("UI: [%s]", text);
#endif
}

/**
 * @brief Display AI response text (complete message)
 * @param text The text to display
 */
void appDisplaySetAIText(const char *text)
{
    if (!gReady || !text) return;
#if defined(UI_TYPE) && (UI_TYPE == BOT_UI_USER)
    if (strlen(text) > 0) {
        lv_label_set_text(sg_lblAI, text);
        lv_obj_clear_flag(sg_aiBubble, LV_OBJ_FLAG_HIDDEN);
        lv_obj_scroll_to_y(sg_chatArea, LV_COORD_MAX, LV_ANIM_ON);
    }
#else
    // TuyaAI.UI.displayText(text, 0);
    PR_NOTICE("AI: [%s]", text);
#endif
}

/**
 * @brief Start AI response stream display
 */
void appDisplayStreamStart(void)
{
#if defined(UI_TYPE) && (UI_TYPE == BOT_UI_USER)
    if (!gReady) return;
    sg_streamBuf[0] = '\0';
    sg_streamLen = 0;
    lv_label_set_text(sg_lblAI, "...");
    lv_obj_clear_flag(sg_aiBubble, LV_OBJ_FLAG_HIDDEN);
#endif
}

/**
 * @brief Append text to AI response stream
 * @param text The text chunk to append
 */
void appDisplayStreamAppend(const char *text)
{
    if (!gReady || !text) return;
#if defined(UI_TYPE) && (UI_TYPE == BOT_UI_USER)
    int chunkLen = strlen(text);
    if (chunkLen > 0 && sg_streamLen + chunkLen < (int)sizeof(sg_streamBuf) - 1) {
        strcat(sg_streamBuf, text);
        sg_streamLen += chunkLen;
        lv_label_set_text(sg_lblAI, sg_streamBuf);
        lv_obj_scroll_to_y(sg_chatArea, LV_COORD_MAX, LV_ANIM_OFF);
    }
#else
    // TuyaAI.UI.displayMessage(UI_DISP_AI_MSG_STREAM_DATA, (uint8_t *)text, strlen(text));
    PR_NOTICE("AI Stream: [%s]", text);
#endif
}

/**
 * @brief End AI response stream display
 */
void appDisplayStreamEnd(void)
{
#if defined(UI_TYPE) && (UI_TYPE == BOT_UI_USER)
    // Final scroll to show complete message
    lv_obj_scroll_to_y(sg_chatArea, LV_COORD_MAX, LV_ANIM_ON);
#endif
}

/**
 * @brief Set status text
 * @param status The status string to display
 */
void appDisplaySetStatus(const char *status)
{
    if (!gReady || !status) return;
#if defined(UI_TYPE) && (UI_TYPE == BOT_UI_USER)
    lv_label_set_text(sg_lblStatus, status);
#else
    TuyaAI.UI.displayMessage(AI_UI_DISP_STATUS, (uint8_t *)status, strlen(status));
    PR_NOTICE("Status: [%s]", status);
#endif
}

/**
 * @brief Set chat mode display
 * @param mode Chat mode (0=HOLD, 1=ONESHOT, 2=WAKEUP, 3=FREE)
 */
void appDisplaySetMode(int mode)
{   
    if (!gReady) return;
    const char *modeStr;
    switch (mode) {
        case AI_CHAT_MODE_HOLD:     modeStr = HOLD_TALK; break;
        case AI_CHAT_MODE_ONE_SHOT: modeStr = TRIG_TALK; break;
        case AI_CHAT_MODE_WAKEUP:   modeStr = WAKEUP_TALK; break;
        case AI_CHAT_MODE_FREE:     modeStr = FREE_TALK; break;
        default:                    modeStr = "---"; break;
    }
#if defined(UI_TYPE) && (UI_TYPE == BOT_UI_USER)
    lv_label_set_text(sg_lblMode, modeStr);
#else
    // TuyaAI.UI.displayMessage(AI_UI_DISP_CHAT_MODE, (uint8_t *)modeStr, strlen(modeStr));
    PR_NOTICE("Mode: [%s]", modeStr);
#endif
}

/**
 * @brief Set WiFi status indicator
 * @param status WiFi status (0=disconnected, 1=good, 2=fair, 3=weak)
 */
void appDisplaySetWifi(AI_UI_WIFI_STATUS_E status)
{
    if (!gReady) return;
#if defined(UI_TYPE) && (UI_TYPE == BOT_UI_USER)
    const char *icon = TuyaAI.UI.getWifiIcon(status);
    if (sg_lblWifi && icon) {
        lv_label_set_text(sg_lblWifi, icon);
    }
#else
    TuyaAI.UI.displayMessage(AI_UI_DISP_NETWORK, (uint8_t *)&status, sizeof(status));
#endif
}

/***********************************************************
***********************static implementations***************
***********************************************************/

#if defined(UI_TYPE) && (UI_TYPE == BOT_UI_USER)

/**
 * @brief Create the minimalist UI layout
 */
static void _createUI(void)
{
    lv_vendor_disp_lock();
    // Get font resources from TuyaAI.UI
    lv_font_t *textFont = TuyaAI.UI.getTextFont();
    lv_font_t *iconFont = TuyaAI.UI.getIconFont();
    
    // ============ Main Screen ============
    sg_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(sg_screen, lv_color_hex(CLR_BG_DARK), 0);
    lv_obj_set_style_pad_all(sg_screen, 0, 0);
    lv_obj_set_style_text_font(sg_screen, textFont, 0);
    lv_obj_set_style_text_color(sg_screen, lv_color_hex(CLR_TEXT_PRIMARY), 0);
    
    // ============ Header Bar ============
    sg_header = lv_obj_create(sg_screen);
    lv_obj_set_size(sg_header, LV_HOR_RES, HEADER_H);
    lv_obj_align(sg_header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(sg_header, lv_color_hex(CLR_HEADER_BG), 0);
    lv_obj_set_style_border_width(sg_header, 0, 0);
    lv_obj_set_style_radius(sg_header, 0, 0);
    lv_obj_set_style_pad_hor(sg_header, PADDING * 2, 0);
    lv_obj_clear_flag(sg_header, LV_OBJ_FLAG_SCROLLABLE);
    
    // Mode label (left side)
    sg_lblMode = lv_label_create(sg_header);
    lv_obj_set_style_text_color(sg_lblMode, lv_color_hex(CLR_ACCENT), 0);
    lv_obj_align(sg_lblMode, LV_ALIGN_LEFT_MID, 0, 0);
    lv_label_set_text(sg_lblMode, WAKEUP_TALK);
    
    // WiFi icon (right side)
    sg_lblWifi = lv_label_create(sg_header);
    lv_obj_set_style_text_font(sg_lblWifi, iconFont, 0);
    lv_obj_set_style_text_color(sg_lblWifi, lv_color_hex(CLR_TEXT_PRIMARY), 0);
    lv_obj_align(sg_lblWifi, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_label_set_text(sg_lblWifi, TuyaAI.UI.getWifiIcon(AI_UI_WIFI_STATUS_DISCONNECTED));
    
    // ============ Status Box ============
    sg_statusBox = lv_obj_create(sg_screen);
    lv_obj_set_size(sg_statusBox, LV_HOR_RES - PADDING * 4, STATUS_H);
    lv_obj_align(sg_statusBox, LV_ALIGN_TOP_MID, 0, HEADER_H + PADDING);
    lv_obj_set_style_bg_color(sg_statusBox, lv_color_hex(CLR_STATUS_BG), 0);
    lv_obj_set_style_border_width(sg_statusBox, 0, 0);
    lv_obj_set_style_radius(sg_statusBox, 12, 0);
    lv_obj_clear_flag(sg_statusBox, LV_OBJ_FLAG_SCROLLABLE);
    
    // Status label (centered)
    sg_lblStatus = lv_label_create(sg_statusBox);
    lv_obj_set_style_text_color(sg_lblStatus, lv_color_hex(CLR_TEXT_PRIMARY), 0);
    lv_obj_center(sg_lblStatus);
    lv_label_set_text(sg_lblStatus, INITIALIZING);
    
    // ============ Chat Area ============
    int chatY = HEADER_H + STATUS_H + PADDING * 3;
    sg_chatArea = lv_obj_create(sg_screen);
    lv_obj_set_size(sg_chatArea, LV_HOR_RES, LV_VER_RES - chatY);
    lv_obj_align(sg_chatArea, LV_ALIGN_TOP_MID, 0, chatY);
    lv_obj_set_style_bg_opa(sg_chatArea, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(sg_chatArea, 0, 0);
    lv_obj_set_style_pad_all(sg_chatArea, PADDING, 0);
    lv_obj_set_flex_flow(sg_chatArea, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(sg_chatArea, PADDING, 0);
    lv_obj_set_scroll_dir(sg_chatArea, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(sg_chatArea, LV_SCROLLBAR_MODE_AUTO);
    // Scrollbar style using LV_PART_SCROLLBAR selector
    lv_obj_set_style_bg_color(sg_chatArea, lv_color_hex(CLR_ACCENT), LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_opa(sg_chatArea, LV_OPA_COVER, LV_PART_SCROLLBAR);
    lv_obj_set_style_width(sg_chatArea, 4, LV_PART_SCROLLBAR);
    
    // ============ User Message Bubble (right aligned) ============
    sg_userBubble = lv_obj_create(sg_chatArea);
    lv_obj_set_size(sg_userBubble, LV_PCT(BUBBLE_MAX_W), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(sg_userBubble, lv_color_hex(CLR_USER_BG), 0);
    lv_obj_set_style_radius(sg_userBubble, BUBBLE_RADIUS, 0);
    lv_obj_set_style_pad_all(sg_userBubble, 12, 0);
    lv_obj_set_style_border_width(sg_userBubble, 0, 0);
    lv_obj_clear_flag(sg_userBubble, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_align(sg_userBubble, LV_ALIGN_RIGHT_MID, 0);
    
    sg_lblUser = lv_label_create(sg_userBubble);
    lv_obj_set_width(sg_lblUser, LV_PCT(100));
    lv_obj_set_style_text_color(sg_lblUser, lv_color_black(), 0);
    lv_label_set_long_mode(sg_lblUser, LV_LABEL_LONG_WRAP);
    lv_label_set_text(sg_lblUser, "");
    
    lv_obj_add_flag(sg_userBubble, LV_OBJ_FLAG_HIDDEN);
    
    // ============ AI Message Bubble (left aligned) ============
    sg_aiBubble = lv_obj_create(sg_chatArea);
    lv_obj_set_size(sg_aiBubble, LV_PCT(BUBBLE_MAX_W), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(sg_aiBubble, lv_color_hex(CLR_AI_BG), 0);
    lv_obj_set_style_radius(sg_aiBubble, BUBBLE_RADIUS, 0);
    lv_obj_set_style_pad_all(sg_aiBubble, 12, 0);
    lv_obj_set_style_border_width(sg_aiBubble, 0, 0);
    lv_obj_clear_flag(sg_aiBubble, LV_OBJ_FLAG_SCROLLABLE);
    
    sg_lblAI = lv_label_create(sg_aiBubble);
    lv_obj_set_width(sg_lblAI, LV_PCT(100));
    lv_obj_set_style_text_color(sg_lblAI, lv_color_hex(CLR_TEXT_PRIMARY), 0);
    lv_label_set_long_mode(sg_lblAI, LV_LABEL_LONG_WRAP);
    lv_label_set_text(sg_lblAI, "");
    
    lv_obj_add_flag(sg_aiBubble, LV_OBJ_FLAG_HIDDEN);
    
    // Load screen
    lv_scr_load(sg_screen);
    lv_vendor_disp_unlock();
    PR_NOTICE("UI: Custom minimalist UI created");
}

#endif // UI_TYPE == BOT_UI_USER
