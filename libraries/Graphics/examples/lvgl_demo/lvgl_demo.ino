#include "Log.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "tuya_cloud_types.h"

#include "tal_api.h"
#include "tkl_output.h"
#include "tkl_spi.h"
#include "tkl_system.h"
#include "lvgl.h"
#include "demos/lv_demos.h"
#include "examples/lv_examples.h"
#include "lv_vendor.h"
#include "board_com_api.h"

#ifdef __cplusplus
}
#endif

static void anim_x_cb(void * var, int32_t v)
{
    lv_obj_set_x(var, v);
}

static void sw_event_cb(lv_event_t * e)
{
    lv_obj_t * sw = lv_event_get_target(e);
    lv_obj_t * label = lv_event_get_user_data(e);

    if(lv_obj_has_state(sw, LV_STATE_CHECKED)) {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, label);
        lv_anim_set_values(&a, lv_obj_get_x(label), 100);
        lv_anim_set_duration(&a, 500);
        lv_anim_set_exec_cb(&a, anim_x_cb);
        lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
        lv_anim_start(&a);
    }
    else {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, label);
        lv_anim_set_values(&a, lv_obj_get_x(label), -lv_obj_get_width(label));
        lv_anim_set_duration(&a, 500);
        lv_anim_set_exec_cb(&a, anim_x_cb);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
        lv_anim_start(&a);
    }

}

/**
 * Start animation on an event
 */
void lv_example_anim_1(void)
{
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello animations!");
    lv_obj_set_pos(label, 100, 10);

    lv_obj_t * sw = lv_switch_create(lv_screen_active());
    lv_obj_center(sw);
    lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_add_event_cb(sw, sw_event_cb, LV_EVENT_VALUE_CHANGED, label);
}

void setup(void)
{
    Serial.begin(115200);
    Log.begin(1024);
    // tal_log_init(TAL_LOG_LEVEL_DEBUG, 1024, (TAL_LOG_OUTPUT_CB)tkl_log_output);
    /* basic init */

    /*hardware register*/
    board_register_hardware();
    // const void *str;
    static char display_name[] = "display";   // 非 const
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

