/**
 * @file tuya_app_main.cpp
 * @author www.tuya.com
 * @brief tuya_app_main module is used to
 * @version 0.1
 * @date 2024-08-20
 *
 * @copyright Copyright (c) tuya.inc 2024
 *
 */
#include "tuya_iot_config.h"

#include "ArduinoMain.h"

#include "cJSON.h"
#include "tkl_uart.h"

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
static char thread_name[] = "arduino_thread";

/***********************************************************
***********************function define**********************
***********************************************************/
static void __wifi_callback_event(WF_EVENT_E event, void *arg)
{
  return;
}

void app_open_sdk_init(void)
{
  // cJSON init
  cJSON_Hooks hooks = {
    .malloc_fn = tal_malloc,
    .free_fn = tal_free
  };
  cJSON_InitHooks(&hooks);

  // file system init
  tal_kv_cfg_t kv_cfg = {
    .seed = "vmlkasdh93dlvlcy",
    .key = "dflfuap134ddlduq",
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
  // TODO: set country code
  // TODO: use netconn_wifi functions
#if (!defined(ARDUINO_T5) && !defined(ARDUINO_ESP32))
  tal_wifi_init(__wifi_callback_event);
  tal_wifi_set_country_code("CN");
#endif
}

static void ArduinoThread(void *arg)
{

#if defined(ARDUINO_T2)
  extern char get_rx2_flag(void);
  while (get_rx2_flag() == 0) {
    tal_system_sleep(1);
  }
#endif // defined(ARDUINO_T2)

#if (!defined(ARDUINO_LN882H)&& !defined(ARDUINO_ESP32))
  tkl_uart_deinit(TUYA_UART_NUM_0);
#if (!defined(ARDUINO_T3) && !defined(ARDUINO_T5) && !defined(ARDUINO_ESP32))
  tkl_uart_deinit(TUYA_UART_NUM_1); // TODO: close vendor log
#endif
#endif // (!defined(ARDUINO_LN882H))

  app_open_sdk_init();

  ArduinoMain();
}

void tuya_app_main(void)
{
#if (!defined(ARDUINO_ESP32))
  __asm("BL __libc_init_array");
#endif

  THREAD_CFG_T thrd_param = {1024 * 8, THREAD_PRIO_1, thread_name}; // Increased stack size for C++
  tal_thread_create_and_start(&arduino_thrd_hdl, NULL, NULL, ArduinoThread, NULL, &thrd_param);
}
