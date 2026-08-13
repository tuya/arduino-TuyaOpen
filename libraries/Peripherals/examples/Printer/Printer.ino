/**
 * @file Printer.ino
 * @brief Thermal printer example for Tuya IoT devices.
 *
 * This example demonstrates how to use the Printer class to drive an ESC/POS
 * thermal printer: printing text, feeding paper, printing a 1-bit bitmap, and
 * reacting to driver events such as paper-out.
 *
 * Wiring:
 * - DP-48A ESC/POS printer on UART0 at 9600 bps (the board default)
 * - The board registers the device inside board_register_hardware()
 *
 * Vendor build requirement:
 * - CONFIG_TUYA_T5AI_BOARD_PRINTER_DP48=y, which selects ENABLE_PRINTER and
 *   ENABLE_PRINTER_DP48. Without it the driver is not compiled in and open()
 *   returns OPRT_NOT_FOUND.
 *
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 *
 * @note ===================== Only supports TUYA_T5AI platform =====================
 */

#include "Printer.h"
#include "Log.h"
#include "board_com_api.h"

/***********************************************************
************************macro define************************
***********************************************************/
// A 16x16 checkerboard, 1 bit per pixel, 2 bytes per row
static const uint8_t kLogoBitmap[] = {
    0xFF, 0xFF, 0x80, 0x01, 0xBF, 0xFD, 0xA0, 0x05, 0xAF, 0xF5, 0xA8, 0x15, 0xA8, 0x15, 0xAF, 0xF5,
    0xA0, 0x05, 0xBF, 0xFD, 0x80, 0x01, 0xFF, 0xFF, 0x00, 0x00, 0x3C, 0x3C, 0x42, 0x42, 0x00, 0x00,
};
#define LOGO_WIDTH  16
#define LOGO_HEIGHT 16

/***********************************************************
***********************static declarations******************
***********************************************************/
static void printerEventCallback(TDL_PRINTER_EVENT_E event, void *arg);
static void printReceipt(void);

/***********************************************************
***********************public functions*********************
***********************************************************/
void setup()
{
    Serial.begin(115200);
    Log.begin();

    PR_NOTICE("============ Tuya Thermal Printer ==============");
    PR_NOTICE("Compile time:        %s", __DATE__);

    // Registers the printer described by the board configuration
    board_register_hardware();

    Printer.setEventCallback(printerEventCallback, nullptr);

    OPERATE_RET rt = Printer.open();
    if (rt != OPRT_OK) {
        PR_ERR("Printer open failed, rt=%d", rt);
        PR_ERR("Is CONFIG_TUYA_T5AI_BOARD_PRINTER_DP48 enabled in the vendor build?");
        return;
    }

    PR_NOTICE("Printer ready, %u dots per line", (unsigned int)Printer.dotsPerLine());

    printReceipt();
}

void loop()
{
    delay(1000);
}

/***********************************************************
***********************static implementations***************
***********************************************************/

/**
 * @brief Print one job: a heading, two text lines and a bitmap
 */
static void printReceipt(void)
{
    if (Printer.beginJob() != OPRT_OK) {
        PR_ERR("Failed to start the print job");
        return;
    }

    Printer.print("Tuya Arduino\n");
    Printer.print("Thermal printer demo\n");
    Printer.print("--------------------\n");

    Printer.printBitmap(0, LOGO_WIDTH, LOGO_HEIGHT, kLogoBitmap);

    // Clear the tear bar so the printed part can be torn off
    Printer.feed(48);

    if (Printer.endJob() != OPRT_OK) {
        PR_ERR("Failed to finish the print job");
        return;
    }

    PR_NOTICE("Print job done");
}

/**
 * @brief Printer driver events
 */
static void printerEventCallback(TDL_PRINTER_EVENT_E event, void *arg)
{
    (void)arg;

    switch (event) {
    case TDL_PRINTER_EVENT_PAPER_OUT:
        PR_WARN("Printer is out of paper");
        break;

    case TDL_PRINTER_EVENT_PAPER_IN:
        PR_NOTICE("Paper reloaded");
        break;

    case TDL_PRINTER_EVENT_OVERHEATED:
        PR_WARN("Print head overheated, printing is paused");
        break;

    case TDL_PRINTER_EVENT_TEMP_NORMAL:
        PR_NOTICE("Print head back to normal temperature");
        break;

    case TDL_PRINTER_EVENT_ERROR:
        PR_ERR("Printer reported an error");
        break;

    default:
        break;
    }
}
