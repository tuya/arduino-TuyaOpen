/**
 * @file Printer.h
 * @brief Arduino-style Printer class for Tuya IoT devices.
 *
 * This file provides an Arduino-friendly wrapper around the Tuya Driver Layer
 * (TDL) printer subsystem, used to drive ESC/POS thermal printers such as the
 * DP-48A attached over UART.
 *
 * A print job is bracketed by begin()/end():
 *
 *     Printer.open();
 *     Printer.beginJob();
 *     Printer.print("Hello");
 *     Printer.feed(3);
 *     Printer.endJob();
 *
 * The board registers the printer device inside board_register_hardware(), so
 * call that before open().
 *
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __PRINTER_H__
#define __PRINTER_H__

#include <Arduino.h>

#include "tuya_cloud_types.h"
#include "tdl_printer_manage.h"

/***********************************************************
 * TYPEDEF
 ***********************************************************/

/**
 * @brief Printer event callback
 * @param event Event reported by the driver
 * @param arg   User argument passed to setEventCallback()
 */
typedef void (*PrinterEventCallback_t)(TDL_PRINTER_EVENT_E event, void *arg);

/***********************************************************
 * CLASS
 ***********************************************************/

/**
 * @class PrinterClass
 * @brief Arduino wrapper around a TDL printer device
 */
class PrinterClass
{
public:
    PrinterClass();
    ~PrinterClass();

    /**
     * @brief Open the printer registered by the board
     * @param name Printer device name; defaults to the board's PRINTER_NAME
     * @return OPRT_OK on success, OPRT_NOT_FOUND if no such device
     */
    OPERATE_RET open(const char *name = nullptr);

    /**
     * @brief Close the printer and release its resources
     */
    void close();

    /**
     * @brief Check whether the printer is open
     */
    bool isOpen();

    /**
     * @brief Install a callback for printer driver events
     * @param callback Callback, or nullptr to remove
     * @param arg      User argument handed back to the callback
     * @note Must be called before open() to take effect.
     */
    void setEventCallback(PrinterEventCallback_t callback, void *arg = nullptr);

    /**
     * @brief Start a print job
     * @return OPRT_OK on success
     */
    OPERATE_RET beginJob();

    /**
     * @brief Finish the current print job
     * @return OPRT_OK on success
     * @note Feeds PRINTER_TEAR_FEED_LINES rows afterwards when configured.
     */
    OPERATE_RET endJob();

    /**
     * @brief Print a line of text
     * @param text NUL-terminated string
     * @return OPRT_OK on success
     */
    OPERATE_RET print(const char *text);

    /**
     * @brief Print a raw byte stream (ESC/POS commands or pre-encoded data)
     * @param data Bytes to send
     * @param len  Number of bytes
     * @return OPRT_OK on success
     */
    OPERATE_RET write(const uint8_t *data, uint32_t len);

    /**
     * @brief Print a 1-bit-per-pixel bitmap
     * @param x      Horizontal offset in dots
     * @param width  Bitmap width in dots
     * @param height Bitmap height in dots
     * @param data   Bitmap rows, (width + 7) / 8 bytes per row
     * @return OPRT_OK on success
     * @note Content past the right edge is clipped by the driver.
     */
    OPERATE_RET printBitmap(uint16_t x, uint16_t width, uint16_t height, const uint8_t *data);

    /**
     * @brief Advance the paper
     * @param lines Number of dot rows to feed
     * @return OPRT_OK on success
     */
    OPERATE_RET feed(uint32_t lines);

    /**
     * @brief Read the printer geometry
     * @param info Receives dots_per_line / bytes_per_line
     * @return OPRT_OK on success
     */
    OPERATE_RET getInfo(TDL_PRINTER_DEV_INFO_T *info);

    /**
     * @brief Printable width in dots, or 0 when unavailable
     */
    uint32_t dotsPerLine();

    /**
     * @brief Forward a driver event to the user callback
     * @note Public only so the C trampoline can reach it; not part of the API.
     */
    void _dispatchEvent(TDL_PRINTER_EVENT_E event);

private:
    TDL_PRINTER_HANDLE     _handle;
    bool                   _open;
    PrinterEventCallback_t _callback;
    void                  *_callbackArg;
};

// Global instance
extern PrinterClass Printer;

#endif /* __PRINTER_H__ */
