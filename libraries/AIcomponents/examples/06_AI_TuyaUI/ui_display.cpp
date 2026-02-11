/**
 * @file ui_display.cpp
 * @brief UI display implementation
 */

#include "ui_display.h"
#include "TuyaAI.h"
#include "Log.h"
#include "lv_vendor.h"

#if defined(ENABLE_LIBLVGL) && (ENABLE_LIBLVGL == 1)
#include "lvgl.h"
static lv_obj_t *gLabelUser = NULL;
static lv_obj_t *gLabelAI = NULL;
static lv_obj_t *gLabelStatus = NULL;
static lv_obj_t *gLabelWifi = NULL;
#endif

static char gAIBuf[1024];
static int gAILen = 0;

void uiInit(void)
{
    lv_vendor_disp_lock();
#if defined(ENABLE_LIBLVGL) && (ENABLE_LIBLVGL == 1)
    lv_font_t *font = TuyaAI.UI.getTextFont();
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x1a1a2e), 0);
    
    // WiFi icon
    gLabelWifi = lv_label_create(scr);
    lv_obj_set_style_text_color(gLabelWifi, lv_color_hex(0x888888), 0);
    lv_label_set_text(gLabelWifi, LV_SYMBOL_WIFI);
    lv_obj_align(gLabelWifi, LV_ALIGN_TOP_RIGHT, -10, 10);
    
    // Status
    gLabelStatus = lv_label_create(scr);
    if (font) lv_obj_set_style_text_font(gLabelStatus, font, 0);
    lv_obj_set_style_text_color(gLabelStatus, lv_color_hex(0x00ff88), 0);
    lv_label_set_text(gLabelStatus, "Ready");
    lv_obj_align(gLabelStatus, LV_ALIGN_TOP_MID, 0, 10);
    
    // User text
    gLabelUser = lv_label_create(scr);
    if (font) lv_obj_set_style_text_font(gLabelUser, font, 0);
    lv_obj_set_style_text_color(gLabelUser, lv_color_hex(0x4fc3f7), 0);
    lv_obj_set_width(gLabelUser, lv_pct(90));
    lv_label_set_long_mode(gLabelUser, LV_LABEL_LONG_WRAP);
    lv_label_set_text(gLabelUser, "User: ...");
    lv_obj_align(gLabelUser, LV_ALIGN_TOP_LEFT, 10, 50);
    
    // AI text
    gLabelAI = lv_label_create(scr);
    if (font) lv_obj_set_style_text_font(gLabelAI, font, 0);
    lv_obj_set_style_text_color(gLabelAI, lv_color_hex(0xffffff), 0);
    lv_obj_set_width(gLabelAI, lv_pct(90));
    lv_label_set_long_mode(gLabelAI, LV_LABEL_LONG_WRAP);
    lv_label_set_text(gLabelAI, "AI: ...");
    lv_obj_align(gLabelAI, LV_ALIGN_CENTER, 0, 20);
#endif
    lv_vendor_disp_unlock();
    PR_DEBUG("UI initialized");
}

void uiSetUser(const char *text)
{
#if defined(ENABLE_LIBLVGL) && (ENABLE_LIBLVGL == 1)
    if (gLabelUser && text) {
        char buf[256];
        snprintf(buf, sizeof(buf), "User: %s", text);
        lv_label_set_text(gLabelUser, buf);
    }
#endif
}

void uiSetAI(const char *text)
{
#if defined(ENABLE_LIBLVGL) && (ENABLE_LIBLVGL == 1)
    if (gLabelAI && text) {
        char buf[512];
        snprintf(buf, sizeof(buf), "AI: %s", text);
        lv_label_set_text(gLabelAI, buf);
    }
#endif
}

void uiSetStatus(const char *text)
{
#if defined(ENABLE_LIBLVGL) && (ENABLE_LIBLVGL == 1)
    if (gLabelStatus && text) {
        lv_label_set_text(gLabelStatus, text);
    }
#endif
    TuyaAI.UI.displayText(text, false);
}

void uiSetWifi(bool connected)
{
#if defined(ENABLE_LIBLVGL) && (ENABLE_LIBLVGL == 1)
    if (gLabelWifi) {
        lv_obj_set_style_text_color(gLabelWifi, 
            lv_color_hex(connected ? 0x00ff88 : 0x888888), 0);
    }
#endif
    TuyaAI.UI.displayWifiStatus(connected ? AI_UI_WIFI_STATUS_GOOD : AI_UI_WIFI_STATUS_DISCONNECTED);
}

void uiResetAIBuffer(void)
{
    gAILen = 0;
    gAIBuf[0] = '\0';
}

void uiAppendAIBuffer(const uint8_t *data, uint32_t len)
{
    if (!data || len == 0) return;
    int remain = sizeof(gAIBuf) - gAILen - 1;
    int copy = ((int)len < remain) ? (int)len : remain;
    if (copy > 0) {
        memcpy(gAIBuf + gAILen, data, copy);
        gAILen += copy;
        gAIBuf[gAILen] = '\0';
        uiSetAI(gAIBuf);
    }
}

void uiFlushAIBuffer(void)
{
    Serial.println("[AI] Complete");
}
