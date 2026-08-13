/**
 * @file TuyaVideo.cpp
 * @author Tuya Inc.
 * @brief TuyaVideoClass implementation - AI video component wrapper
 *
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 *
 */
#include "TuyaVideo.h"

/***********************************************************
***********************TuyaVideoClass Implementation********
***********************************************************/

TuyaVideoClass::TuyaVideoClass()
{
    _initialized = false;
    _started     = false;
}

TuyaVideoClass::~TuyaVideoClass()
{
    if (_initialized) {
        end();
    }
}

OPERATE_RET TuyaVideoClass::begin()
{
    if (_initialized) {
        return OPRT_OK;
    }

    OPERATE_RET rt = ai_video_init();
    if (rt == OPRT_OK) {
        _initialized = true;
    }
    return rt;
}

void TuyaVideoClass::end()
{
    if (!_initialized) {
        return;
    }

    if (_started) {
        stop();
    }

    /* Upstream ai_video_input.h has no deinit symbol, so there is nothing
     * further to release - just clear the local initialized flag. */
    _initialized = false;
}

bool TuyaVideoClass::isInitialized()
{
    return _initialized;
}

OPERATE_RET TuyaVideoClass::start()
{
    if (!_initialized) {
        return OPRT_INVALID_PARM;
    }

    OPERATE_RET rt = ai_video_start();
    if (rt == OPRT_OK) {
        _started = true;
    }
    return rt;
}

OPERATE_RET TuyaVideoClass::stop()
{
    if (!_initialized) {
        return OPRT_INVALID_PARM;
    }

    OPERATE_RET rt = ai_video_stop();
    if (rt == OPRT_OK) {
        _started = false;
    }
    return rt;
}

OPERATE_RET TuyaVideoClass::getJpegFrame(uint8_t **data, uint32_t *len)
{
    if (!_initialized || data == nullptr || len == nullptr) {
        return OPRT_INVALID_PARM;
    }

    return ai_video_get_jpeg_frame(data, len);
}

OPERATE_RET TuyaVideoClass::freeJpegFrame(uint8_t **data)
{
    if (!_initialized || data == nullptr) {
        return OPRT_INVALID_PARM;
    }

    return ai_video_jpeg_image_free(data);
}

OPERATE_RET TuyaVideoClass::setYuvFrameCallback(AI_VIDEO_FLUSH_CB cb)
{
    if (!_initialized || cb == nullptr) {
        return OPRT_INVALID_PARM;
    }

    return ai_video_set_yuv_frame_flush_cb(cb);
}
