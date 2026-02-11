/**
 * @file appMCP.cpp
 * @brief MCP (Model Context Protocol) tools for AI ChatBot
 *
 * Demonstrates MCP tool registration using TUYA_MCP_TOOL_ADD macro.
 * This provides the same functionality as the C API's AI_MCP_TOOL_ADD
 * but can be used directly in C++ code.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#include "AppMCP.h"
#include "TuyaAI.h"
#include "cJSON.h"

// Include Camera for image capture tool
#include "dvpCamera.h"
#include "Display.h"
/***********************************************************
************************macro define************************
***********************************************************/
#define MCP_SERVER_NAME     "Tuya MCP Server"
#define MCP_SERVER_VERSION  "1.0"
#define CAMERA_FPS          15
/***********************************************************
***********************variable define**********************
***********************************************************/
static Camera gCamera;
static Display gDisplay;
/***********************************************************
***********************MCP Tool Callbacks*******************
***********************************************************/
static void mcpRegisterExampleTool_1(void);
static void mcpRegisterExampleTool_2(void);
static void mcpRegisterExampleTool_3(void);
static void mcpRegisterExampleTool_4(void);

static OPERATE_RET onGetDeviceInfo(MCPPropertyList_t properties, MCPReturnValue_t retVal, void *userData);
static OPERATE_RET onSetVolume(MCPPropertyList_t properties, MCPReturnValue_t retVal, void *userData);
static OPERATE_RET onTakePhoto(MCPPropertyList_t properties, MCPReturnValue_t retVal, void *userData);
static OPERATE_RET onSetMode(MCPPropertyList_t properties, MCPReturnValue_t retVal, void *userData);
/***********************************************************
***********************MCP Initialization*******************
***********************************************************/

static OPERATE_RET mcpRegisterTool(void *data) {
    OPERATE_RET rt = TuyaAI.MCP.begin();
    if (rt != OPRT_OK) {
        PR_ERR("Failed to initialize MCP server: %d", rt);
        return rt;
    }

    mcpRegisterExampleTool_1();
    mcpRegisterExampleTool_2();
    mcpRegisterExampleTool_3();
    mcpRegisterExampleTool_4();
    
    PR_INFO("MCP tools registered successfully");
    return OPRT_OK;
}

/**
 * @brief Initialize and register all MCP tools
 * @return OPRT_OK on success, error code on failure
 * 
 * This function demonstrates MCP tool registration using C++ helper functions.
 */
OPERATE_RET appMCPInit(void) {
    PR_DEBUG("appMCPInit called");
    OPERATE_RET rt = OPRT_OK;
    rt = tal_event_subscribe(EVENT_MQTT_CONNECTED, "mcpRegisterTool", mcpRegisterTool, SUBSCRIBE_TYPE_ONETIME);
    if (rt != OPRT_OK) {
        PR_ERR("Failed to subscribe to MQTT event: %d", rt);
        return rt;
    }

    // Initialize display
    if (OPRT_OK != gDisplay.begin()) {
        PR_ERR("Failed to initialize display: %d", rt);
    }
    PR_DEBUG("Display initialized");

    // Initialize camera
    if (OPRT_OK != gCamera.begin(CameraResolution::RES_480X480, CAMERA_FPS, 
                      CameraFormat::JPEG_YUV422, EncodingQuality::LOW)) {
        PR_ERR("Failed to initialize camera");
    }
    PR_DEBUG("Camera initialized");
    return rt;
}

/**
 * @brief Deinitialize MCP
 */
void appMCPDeinit(void) {
    TuyaAI.MCP.end();
    PR_INFO("MCP deinitialized");
}

/**
 * @brief Get device information callback
 */
static OPERATE_RET onGetDeviceInfo(MCPPropertyList_t properties, 
                            MCPReturnValue_t retVal, 
                            void *userData) {
    PR_DEBUG("Getting device info");
    cJSON *json = cJSON_CreateObject();
    if (!json) {
        PR_ERR("Create JSON object failed");
        return OPRT_MALLOC_FAILED;
    }
    
    // Add device information
    cJSON_AddStringToObject(json, "model", PROJECT_NAME);
    cJSON_AddStringToObject(json, "serialNumber", "123456789");
    cJSON_AddStringToObject(json, "firmwareVersion", PROJECT_VERSION);
    
    // Set return value
    TuyaAI.MCP.setReturnJson(retVal, json);
    PR_DEBUG("Device info returned");
    
    return OPRT_OK;
}

/**
 * @brief Take photo callback
 */
static OPERATE_RET onTakePhoto(MCPPropertyList_t properties, 
                        MCPReturnValue_t retVal, 
                        void *userData) {
    PR_DEBUG("Taking photo");
    
    TuyaAI.UI.disableUpdateLVGL();

    CameraFrame jpegFrame;
    CameraFrame yuv422Frame;
    gDisplay.fillScreen(0x000000);
    uint8_t time = 50;
    while (1) {
        gCamera.getFrame(yuv422Frame, CameraFormat::YUV422, 100);   
        gDisplay.displayYUV422Frame(yuv422Frame.data, yuv422Frame.width, yuv422Frame.height);
        PR_DEBUG("YUV422 Camera width: %d, height: %d, dataLen: %d", 
                 yuv422Frame.width, yuv422Frame.height, yuv422Frame.dataLen);
                 PR_DEBUG("time =%d", time);
        if (time-- == 0) {
            break;
        }
        delay(10);
    }
    gCamera.getFrame(jpegFrame, CameraFormat::JPEG, 100);
    PR_DEBUG("JPEG Camera width: %d, height: %d, dataLen: %d", 
                jpegFrame.width, jpegFrame.height, jpegFrame.dataLen);
    TuyaAI.MCP.setReturnImage(retVal, MCP_MIME_JPEG, jpegFrame.data, jpegFrame.dataLen);
    gDisplay.fillScreen(0x000000);
    TuyaAI.UI.enableUpdateLVGL();
    PR_DEBUG("Photo taken");
    return OPRT_OK;
}

/**
 * @brief Set volume callback
 */
static OPERATE_RET onSetVolume(MCPPropertyList_t properties, 
                        MCPReturnValue_t retVal, 
                        void *userData) {
    // Get volume from properties
    int volume = TuyaAI.MCP.getPropertyInt(properties, "volume", 50);
    PR_DEBUG("Setting volume to %d", volume);
    // Set volume using TuyaAI Audio component
    TuyaAI.Audio.setVolume(volume);
    // set return value
    TuyaAI.MCP.setReturnBool(retVal, true);
    return OPRT_OK;
}

/**
 * @brief Set chat mode callback
 */
static OPERATE_RET onSetMode(MCPPropertyList_t properties, 
                      MCPReturnValue_t retVal, 
                      void *userData) {
    // Get mode from properties
    int mode = TuyaAI.MCP.getPropertyInt(properties, "mode", 0);
    PR_NOTICE("Setting chat mode to %d", mode);
    
    TuyaAI.setChatMode((AI_CHAT_MODE_E)mode);
    
    TuyaAI.MCP.setReturnBool(retVal, true);
    return OPRT_OK;
}

static void mcpRegisterExampleTool_1(void) {
    // Register tool without properties
    TUYA_MCP_TOOL_ADD_SIMPLE(
        "device_info_get",
        "Get device information such as model, serial number, and firmware version.",
        onGetDeviceInfo,
        nullptr
    );
    PR_INFO("MCP: device_info_get tool registered");
}

static void mcpRegisterExampleTool_2(void) {
    // Register tool with multiple properties
    TuyaMCPPropDef photoProps[] = {
        TuyaMCPPropStr("question", "The question prompting the photo capture."),
        TuyaMCPPropIntDefRange("count", "Number of photos to capture (1-10).", 1, 1, 10)
    };
    TUYA_MCP_TOOL_REGISTER(
        "device_camera_take_photo",
        "Activates the device's camera to capture one or more photos.\n"
        "Parameters:\n"
        "- count (int): Number of photos to capture (1-10).\n"
        "Response:\n"
        "- Returns the captured photos encoded in Base64 format.",
        onTakePhoto,
        nullptr,
        photoProps, 2
    );
    PR_INFO("MCP: device_camera_take_photo tool registered");
}

static void mcpRegisterExampleTool_3(void) { 
    // Register tool with integer property (range 0-100)
    TuyaMCPPropDef volumeProps[] = {
        TuyaMCPPropIntRange("volume", "The volume level to set (0-100).", 0, 100)
    };
    TUYA_MCP_TOOL_REGISTER(
        "device_audio_volume_set",
        "Sets the device's volume level.\n"
        "Parameters:\n"
        "- volume (int): The volume level to set (0-100).\n"
        "Response:\n"
        "- Returns true if the volume was set successfully.",
        onSetVolume,
        nullptr,
        volumeProps, 1
    );
    PR_INFO("MCP: device_audio_volume_set tool registered");
}

static void mcpRegisterExampleTool_4(void) { 
    // Register tool with mode property
    TuyaMCPPropDef modeProps[] = {
        TuyaMCPPropIntRange("mode", "The chat mode (0=hold, 1=key_press, 2=wakeup, 3=free)", 0, 3)
    };
    TUYA_MCP_TOOL_REGISTER(
        "device_audio_mode_set",
        "Sets the device's chat mode.\n"
        "Parameters:\n"
        "- mode (integer): The chat mode (0=hold, 1=key_press, 2=wakeup, 3=free).\n"
        "Response:\n"
        "- Returns true if the mode was set successfully.",
        onSetMode,
        nullptr,
        modeProps, 1
    );
    PR_INFO("MCP: device_audio_mode_set tool registered");
}