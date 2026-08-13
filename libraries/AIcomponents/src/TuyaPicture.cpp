/**
 * @file TuyaPicture.cpp
 * @brief TuyaPicture class implementation
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 */

#include "TuyaPicture.h"
#include <string.h>

/***********************************************************
***********************class implementation*****************
***********************************************************/

TuyaPictureClass::TuyaPictureClass()
{
    _initialized = false;
}

TuyaPictureClass::~TuyaPictureClass() {}

OPERATE_RET TuyaPictureClass::begin()
{
    if (_initialized) {
        return OPRT_OK;
    }

    OPERATE_RET rt = ai_picture_init();
    if (rt != OPRT_OK) {
        return rt;
    }

    _initialized = true;
    return OPRT_OK;
}

void TuyaPictureClass::end()
{
    // Upstream ai_picture has no deinit symbol; just clear the flag.
    _initialized = false;
}

bool TuyaPictureClass::isInitialized()
{
    return _initialized;
}

const char *TuyaPictureClass::albumName()
{
    return ai_picture_get_album_name();
}

OPERATE_RET TuyaPictureClass::saveToAlbum(const uint8_t *data, uint32_t len, const char *hint, char *outName,
                                          uint32_t outNameSize)
{
    if (!_initialized || data == nullptr || len == 0) {
        return OPRT_INVALID_PARM;
    }

    // Upstream expects a fixed-size array of AI_PICTURE_NAME_MAX_LEN + 1
    // bytes and writes up to that size; passing a caller-supplied shorter
    // buffer directly would overflow it, so always use a local buffer here.
    char nameBuf[AI_PICTURE_NAME_MAX_LEN + 1] = {0};

    OPERATE_RET rt = ai_picture_save_to_album(const_cast<uint8_t *>(data), len, hint, nameBuf);
    if (rt != OPRT_OK) {
        return rt;
    }

    if (outName != nullptr && outNameSize > 0) {
        strncpy(outName, nameBuf, outNameSize - 1);
        outName[outNameSize - 1] = '\0';
    }

    return OPRT_OK;
}

OPERATE_RET TuyaPictureClass::recognize(const uint8_t *data, uint32_t len)
{
    if (!_initialized || data == nullptr || len == 0) {
        return OPRT_INVALID_PARM;
    }

    return ai_picture_input_recognize(const_cast<uint8_t *>(data), len);
}

OPERATE_RET TuyaPictureClass::attachFromAlbum(const char *filename, const char *text)
{
    if (!_initialized || filename == nullptr) {
        return OPRT_INVALID_PARM;
    }

    return ai_picture_input_add_from_album(const_cast<char *>(filename), const_cast<char *>(text));
}

OPERATE_RET TuyaPictureClass::detachFromAlbum(const char *filename)
{
    if (!_initialized || filename == nullptr) {
        return OPRT_INVALID_PARM;
    }

    return ai_picture_input_del_from_album(const_cast<char *>(filename));
}

OPERATE_RET TuyaPictureClass::sendAttachments()
{
    if (!_initialized) {
        return OPRT_INVALID_PARM;
    }

    return ai_picture_input_from_album();
}

OPERATE_RET TuyaPictureClass::setOutputSize(uint16_t width, uint16_t height)
{
    if (!_initialized || width == 0 || height == 0) {
        return OPRT_INVALID_PARM;
    }

    return ai_picture_output_set_size(width, height);
}

OPERATE_RET TuyaPictureClass::beginOutputDownload(uint16_t width, uint16_t height)
{
    if (!_initialized || width == 0 || height == 0) {
        return OPRT_INVALID_PARM;
    }

    // Upstream only declares this entry point when the download path is built
    // in, so keep the Arduino API stable and report it as unsupported instead.
#if defined(ENABLE_COMP_AI_PICTURE_HOSTING_DLD) && (ENABLE_COMP_AI_PICTURE_HOSTING_DLD == 1)
    return ai_picture_output_dld_init(width, height);
#else
    return OPRT_NOT_SUPPORTED;
#endif
}
