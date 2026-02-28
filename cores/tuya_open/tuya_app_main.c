/**
 * @file tuya_app_main.c
 * @brief tuya_app_main module
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 */
#include "tuya_iot_config.h"

#include "ArduinoMain.h"

#include "cJSON.h"
#include "tkl_uart.h"
#include "netmgr.h"
#if defined(ENABLE_WIFI) && (ENABLE_WIFI == 1)
#include "netconn_wifi.h"
#endif
#if defined(ENABLE_WIRED) && (ENABLE_WIRED == 1)
#include "netconn_wired.h"
#endif
#if defined(ENABLE_LIBLWIP) && (ENABLE_LIBLWIP == 1)
#include "lwip_init.h"
#endif

// Provide _init and _fini functions for C++ constructor/destructor initialization
void _init(void) __attribute__((weak));
void _fini(void) __attribute__((weak));

void _init(void)
{
    // Empty implementation - C++ constructors will be called by __libc_init_array
}

void _fini(void)
{
    // Empty implementation - C++ destructors cleanup
}

/***********************************************************
************************macro define************************
***********************************************************/

/***********************************************************
***********************typedef define***********************
***********************************************************/

/***********************************************************
********************function declaration********************
***********************************************************/

/***********************************************************
***********************variable define**********************
***********************************************************/
static THREAD_HANDLE arduino_thrd_hdl = NULL;
static char          thread_name[]    = "arduino_thread";

/***********************************************************
***********************function define**********************
***********************************************************/
#if (!defined(ARDUINO_CHIP_T5) && !defined(ARDUINO_CHIP_esp32))
static void __wifi_callback_event(WF_EVENT_E event, void *arg)
{
    return;
}
#endif

void app_open_sdk_init(void)
{
    //! open iot development kit runtim init
#if defined(ENABLE_EXT_RAM) && (ENABLE_EXT_RAM == 1)
    cJSON_InitHooks(&(cJSON_Hooks){.malloc_fn = tal_psram_malloc, .free_fn = tal_psram_free});
#else
    cJSON_InitHooks(&(cJSON_Hooks){.malloc_fn = tal_malloc, .free_fn = tal_free});
#endif

    // file system init
    tal_kv_cfg_t kv_cfg = {
        .seed = "vmlkasdh93dlvlcy",
        .key  = "dflfuap134ddlduq",
    };
    tal_kv_init(&kv_cfg);
    // software timer init
    tal_sw_timer_init();
    // work queue init
    tal_workq_init();

    // lwip init
#if defined(ENABLE_LIBLWIP) && (ENABLE_LIBLWIP == 1)
    TUYA_LwIP_Init();
#endif

    // wifi init
#if (!defined(ARDUINO_CHIP_T5) && !defined(ARDUINO_CHIP_esp32))
    tal_wifi_init(__wifi_callback_event);
    tal_wifi_set_country_code("CN");
#endif
}

static void ArduinoThread(void *arg)
{

#if defined(ARDUINO_CHIP_T2)
    extern char get_rx2_flag(void);
    while (get_rx2_flag() == 0) {
        tal_system_sleep(1);
    }
#endif // defined(ARDUINO_CHIP_T2)

#if defined(ARDUINO_CHIP_LN882H) && defined(ARDUINO_CHIP_esp32)
    tkl_uart_deinit(TUYA_UART_NUM_0);
#elif defined(ARDUINO_CHIP_T3) && defined(ARDUINO_CHIP_T5)
    tkl_uart_deinit(TUYA_UART_NUM_1); // TODO: close vendor log
#endif

    app_open_sdk_init();

    ArduinoMain();
}

void tuya_app_main(void)
{
#if (!defined(ARDUINO_CHIP_esp32))
    __asm("BL __libc_init_array");
#endif

    THREAD_CFG_T thrd_param = {1024 * 8, THREAD_PRIO_1, thread_name}; // Increased stack size for C++
    tal_thread_create_and_start(&arduino_thrd_hdl, NULL, NULL, ArduinoThread, NULL, &thrd_param);
}
