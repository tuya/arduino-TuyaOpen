/**
 * @file Printer.cpp
 * @brief Arduino-style Printer class implementation
 *
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 *
 */

#include "Printer.h"

/***********************************************************
 * MACRO
 ***********************************************************/
// Kconfig exposes the board's printer name; fall back to the upstream default.
#ifndef PRINTER_NAME
#define PRINTER_NAME "printer"
#endif

/***********************************************************
 * STATIC
 ***********************************************************/
// Global instance
PrinterClass Printer;

static PrinterClass *_printerInstance = nullptr;

/**
 * @brief Adapt the TDL callback to the Arduino-style one
 */
static void __printerEventHandler(TDL_PRINTER_HANDLE handle, TDL_PRINTER_EVENT_E event, void *data, void *arg)
{
    (void)handle;
    (void)data;

    PrinterClass *self = (PrinterClass *)arg;
    if (self == nullptr) {
        return;
    }
    self->_dispatchEvent(event);
}

/***********************************************************
 * CLASS
 ***********************************************************/

PrinterClass::PrinterClass()
{
    _handle      = nullptr;
    _open        = false;
    _callback    = nullptr;
    _callbackArg = nullptr;

    _printerInstance = this;
}

PrinterClass::~PrinterClass()
{
    if (_open) {
        close();
    }
    _printerInstance = nullptr;
}

void PrinterClass::_dispatchEvent(TDL_PRINTER_EVENT_E event)
{
    if (_callback != nullptr) {
        _callback(event, _callbackArg);
    }
}

OPERATE_RET PrinterClass::open(const char *name)
{
    if (_open) {
        return OPRT_OK;
    }

    OPERATE_RET rt = tdl_printer_find(name != nullptr ? name : PRINTER_NAME, &_handle);
    if (rt != OPRT_OK) {
        return rt;
    }

    TDL_PRINTER_OPEN_PARAM_T param = {0};
    param.event_cb                 = __printerEventHandler;
    param.event_cb_arg             = this;
    param.poll_interval_ms         = 0; // 0 keeps the driver's configured interval

    rt = tdl_printer_open(_handle, &param);
    if (rt != OPRT_OK) {
        _handle = nullptr;
        return rt;
    }

    _open = true;
    return OPRT_OK;
}

void PrinterClass::close()
{
    if (!_open) {
        return;
    }

    tdl_printer_close(_handle);
    _handle = nullptr;
    _open   = false;
}

bool PrinterClass::isOpen()
{
    return _open;
}

void PrinterClass::setEventCallback(PrinterEventCallback_t callback, void *arg)
{
    _callback    = callback;
    _callbackArg = arg;
}

OPERATE_RET PrinterClass::beginJob()
{
    if (!_open) {
        return OPRT_INVALID_PARM;
    }

    return tdl_printer_start(_handle);
}

OPERATE_RET PrinterClass::endJob()
{
    if (!_open) {
        return OPRT_INVALID_PARM;
    }

    return tdl_printer_end(_handle);
}

OPERATE_RET PrinterClass::print(const char *text)
{
    if (!_open || text == nullptr) {
        return OPRT_INVALID_PARM;
    }

    return tdl_printer_send_text(_handle, text);
}

OPERATE_RET PrinterClass::write(const uint8_t *data, uint32_t len)
{
    if (!_open || data == nullptr || len == 0) {
        return OPRT_INVALID_PARM;
    }

    return tdl_printer_send(_handle, data, len);
}

OPERATE_RET PrinterClass::printBitmap(uint16_t x, uint16_t width, uint16_t height, const uint8_t *data)
{
    if (!_open || data == nullptr || width == 0 || height == 0) {
        return OPRT_INVALID_PARM;
    }

    return tdl_printer_send_bitmap(_handle, x, width, height, data);
}

OPERATE_RET PrinterClass::feed(uint32_t lines)
{
    if (!_open) {
        return OPRT_INVALID_PARM;
    }

    return tdl_printer_paper_feed(_handle, lines);
}

OPERATE_RET PrinterClass::getInfo(TDL_PRINTER_DEV_INFO_T *info)
{
    if (!_open || info == nullptr) {
        return OPRT_INVALID_PARM;
    }

    return tdl_printer_get_dev_info(_handle, info);
}

uint32_t PrinterClass::dotsPerLine()
{
    TDL_PRINTER_DEV_INFO_T info = {0};

    if (getInfo(&info) != OPRT_OK) {
        return 0;
    }

    return info.dots_per_line;
}
