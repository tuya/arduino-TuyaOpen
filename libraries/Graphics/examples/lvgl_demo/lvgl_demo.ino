/**
 * @file lvgl_demo.ino
 * @brief lvgl demo
 * This example demonstrates basic usage of the LVGL graphics library
 * including initialization, creating a label, and displaying text.
 * 
 * @note The LVGL version is v9.1.0
 * 
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 * 
 * @note ===================== Only support TUYA_T5AI platform =====================
 */

#include "Log.h"

#include "tuya_cloud_types.h"
#include "tal_api.h"

#include "board_com_api.h"
#include "lvgl.h"
#include "lv_vendor.h"

void setup(void)
{
    // Initialize logging
    Serial.begin(115200);
    Log.begin(1024);

    /*hardware register*/
    board_register_hardware();
    // const void *str;
    static char display_name[] = "display";
    lv_vendor_init(display_name);
    lv_vendor_disp_lock();

    /*Change the active screen's background color*/
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_white(), LV_PART_MAIN);
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello World!");
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_black(), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0); 

    lv_vendor_disp_unlock();
    lv_vendor_start(5, 1024*8);
}
void loop()
{
    delay(10);
}

