/**
 * @file TuyaMCP.h
 * @author Tuya Inc.
 * @brief TuyaMCPClass declaration and MCP-related types
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */
#ifndef __TUYA_MCP_H_
#define __TUYA_MCP_H_

// Include MCP server header for C API
#if defined(ENABLE_COMP_AI_MCP) && (ENABLE_COMP_AI_MCP == 1)
#include "ai_mcp_server.h"
#endif

/***********************************************************
***********************typedef define***********************
***********************************************************/

/**
 * @brief MCP image MIME types
 */
typedef enum {
    MCP_MIME_JPEG = 0,          /**< JPEG image */
    MCP_MIME_PNG,               /**< PNG image */
    MCP_MIME_GIF,               /**< GIF image */
    MCP_MIME_WEBP               /**< WebP image */
} MCPImageMime_t;

/**
 * @brief MCP Tool Return Value (opaque handle)
 */
typedef void* MCPReturnValue_t;

/**
 * @brief MCP Property List (opaque handle)
 */
typedef void* MCPPropertyList_t;

/**
 * @brief MCP Tool callback function type
 * @param properties Property list from AI call
 * @param retVal     Return value to set
 * @param userData   User data pointer
 * @return 0 on success, non-zero on failure
 */
typedef int (*MCPToolCallback_t)(MCPPropertyList_t properties, 
                                  MCPReturnValue_t retVal, 
                                  void *userData);

/***********************************************************
**************C++ MCP Property Helper Functions*************
***********************************************************/

#if defined(ENABLE_COMP_AI_MCP) && (ENABLE_COMP_AI_MCP == 1)

/**
 * @brief C++ compatible property definition structure
 * 
 * This structure is used to pass property definitions to the registration
 * function. Unlike the C API's compound literals, this works in C++.
 */
struct TuyaMCPPropDef {
    const char *name;
    MCP_PROPERTY_TYPE_E type;
    const char *description;
    bool has_default;
    int default_int;
    bool default_bool;
    const char *default_str;
    bool has_range;
    int min_val;
    int max_val;
};

/**
 * @brief Create an integer property definition
 */
inline TuyaMCPPropDef TuyaMCPPropInt(const char *name, const char *desc) {
    TuyaMCPPropDef def = {};
    def.name = name;
    def.type = MCP_PROPERTY_TYPE_INTEGER;
    def.description = desc;
    def.has_default = false;
    def.has_range = false;
    return def;
}

/**
 * @brief Create an integer property definition with default value
 */
inline TuyaMCPPropDef TuyaMCPPropIntDef(const char *name, const char *desc, int defVal) {
    TuyaMCPPropDef def = {};
    def.name = name;
    def.type = MCP_PROPERTY_TYPE_INTEGER;
    def.description = desc;
    def.has_default = true;
    def.default_int = defVal;
    def.has_range = false;
    return def;
}

/**
 * @brief Create an integer property definition with range
 */
inline TuyaMCPPropDef TuyaMCPPropIntRange(const char *name, const char *desc, int minVal, int maxVal) {
    TuyaMCPPropDef def = {};
    def.name = name;
    def.type = MCP_PROPERTY_TYPE_INTEGER;
    def.description = desc;
    def.has_default = false;
    def.has_range = true;
    def.min_val = minVal;
    def.max_val = maxVal;
    return def;
}

/**
 * @brief Create an integer property definition with default value and range
 */
inline TuyaMCPPropDef TuyaMCPPropIntDefRange(const char *name, const char *desc, int defVal, int minVal, int maxVal) {
    TuyaMCPPropDef def = {};
    def.name = name;
    def.type = MCP_PROPERTY_TYPE_INTEGER;
    def.description = desc;
    def.has_default = true;
    def.default_int = defVal;
    def.has_range = true;
    def.min_val = minVal;
    def.max_val = maxVal;
    return def;
}

/**
 * @brief Create a boolean property definition
 */
inline TuyaMCPPropDef TuyaMCPPropBool(const char *name, const char *desc) {
    TuyaMCPPropDef def = {};
    def.name = name;
    def.type = MCP_PROPERTY_TYPE_BOOLEAN;
    def.description = desc;
    def.has_default = false;
    return def;
}

/**
 * @brief Create a boolean property definition with default value
 */
inline TuyaMCPPropDef TuyaMCPPropBoolDef(const char *name, const char *desc, bool defVal) {
    TuyaMCPPropDef def = {};
    def.name = name;
    def.type = MCP_PROPERTY_TYPE_BOOLEAN;
    def.description = desc;
    def.has_default = true;
    def.default_bool = defVal;
    return def;
}

/**
 * @brief Create a string property definition
 */
inline TuyaMCPPropDef TuyaMCPPropStr(const char *name, const char *desc) {
    TuyaMCPPropDef def = {};
    def.name = name;
    def.type = MCP_PROPERTY_TYPE_STRING;
    def.description = desc;
    def.has_default = false;
    return def;
}

/**
 * @brief Create a string property definition with default value
 */
inline TuyaMCPPropDef TuyaMCPPropStrDef(const char *name, const char *desc, const char *defVal) {
    TuyaMCPPropDef def = {};
    def.name = name;
    def.type = MCP_PROPERTY_TYPE_STRING;
    def.description = desc;
    def.has_default = true;
    def.default_str = defVal;
    return def;
}

/**
 * @brief Register an MCP tool with properties (C++ version)
 * 
 * @param name Tool name
 * @param description Tool description
 * @param callback Callback function
 * @param userData User data pointer
 * @param props Array of property definitions
 * @param propCount Number of properties
 * @return OPRT_OK on success, error code on failure
 */
int TuyaMCPToolRegister(const char *name, const char *description,
                        MCPToolCallback_t callback, void *userData,
                        const TuyaMCPPropDef *props, int propCount);

/**
 * @brief Register an MCP tool without properties
 */
inline int TuyaMCPToolRegister(const char *name, const char *description,
                               MCPToolCallback_t callback, void *userData = nullptr) {
    return TuyaMCPToolRegister(name, description, callback, userData, nullptr, 0);
}

#endif // ENABLE_COMP_AI_MCP

/***********************************************************
**************C++ MCP Tool Registration Macros**************
***********************************************************/

/**
 * @defgroup TUYA_MCP_MACROS MCP Tool Registration Macros
 * @brief Simple macros for registering MCP tools in C++
 * @{
 */

#if defined(ENABLE_COMP_AI_MCP) && (ENABLE_COMP_AI_MCP == 1)

/**
 * @brief Register MCP tool with property array
 * 
 * This macro provides a simple way to register MCP tools with properties
 * in C++ code. Unlike the C API, it uses an array-based approach that
 * is compatible with C++.
 * 
 * @param name Tool name string
 * @param description Tool description string
 * @param callback Callback function (MCPToolCallback_t type)
 * @param user_data User data pointer (use nullptr if not needed)
 * @param props_array Array of TuyaMCPPropDef properties
 * @param props_count Number of properties in the array
 * 
 * @return OPRT_OK (0) on success, error code on failure
 * 
 * @example Tool without properties
 * @code
 * TUYA_MCP_TOOL_REGISTER("device_info_get",
 *                        "Get device information",
 *                        onGetDeviceInfo,
 *                        nullptr,
 *                        nullptr, 0);
 * @endcode
 * 
 * @example Tool with properties
 * @code
 * TuyaMCPPropDef volumeProps[] = {
 *     TuyaMCPPropIntRange("volume", "Volume level (0-100)", 0, 100)
 * };
 * TUYA_MCP_TOOL_REGISTER("device_volume_set",
 *                        "Set device volume",
 *                        onSetVolume,
 *                        nullptr,
 *                        volumeProps, 1);
 * @endcode
 */
#define TUYA_MCP_TOOL_REGISTER(name, description, callback, user_data, props_array, props_count) \
    TuyaMCPToolRegister((name), (description), (MCPToolCallback_t)(callback), (user_data), (props_array), (props_count))

/**
 * @brief Shorthand macro for registering tool without properties
 */
#define TUYA_MCP_TOOL_ADD_SIMPLE(name, description, callback, user_data) \
    TuyaMCPToolRegister((name), (description), (MCPToolCallback_t)(callback), (user_data), nullptr, 0)

#else
/* Stub definitions when MCP component is disabled */
#define TUYA_MCP_TOOL_REGISTER(name, description, callback, user_data, props_array, props_count) (-1)
#define TUYA_MCP_TOOL_ADD_SIMPLE(name, description, callback, user_data) (-1)
#endif

/** @} */ // end of TUYA_MCP_MACROS

/***********************************************************
***********************class definition*********************
***********************************************************/

/**
 * @class TuyaMCPClass
 * @brief MCP (Model Context Protocol) server management class
 * 
 * Provides interface for initializing MCP server and registering tools
 * that allow AI to interact with device features.
 * 
 * @note Use the global TuyaAI.MCP instance instead of creating your own.
 */
class TuyaMCPClass {
public:
    TuyaMCPClass();
    ~TuyaMCPClass();
    
    /**
     * @brief Initialize MCP server
     * @param name Server name
     * @param version Server version
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET begin();
    
    /**
     * @brief Deinitialize MCP server
     */
    void end();
    
    /**
     * @brief Check if MCP is initialized
     * @return true if initialized
     */
    bool isInitialized();
    
    /**
     * @brief Check if MCP feature is available
     * @return true if MCP component is enabled
     */
    bool isAvailable();
    
    /**
     * @brief Register MCP tool without properties
     * @param name Tool name
     * @param description Tool description  
     * @param callback Callback function
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET registerTool(const char *name, const char *description,
                      MCPToolCallback_t callback);
    
    /**
     * @brief Register MCP tool with user data (no properties)
     * @param name Tool name
     * @param description Tool description
     * @param callback Callback function
     * @param userData User data pointer
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET registerTool(const char *name, const char *description,
                      MCPToolCallback_t callback, void *userData);
    
    /**
     * @brief Set return value as boolean
     * @param retVal Return value handle
     * @param value  Boolean value
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET setReturnBool(MCPReturnValue_t retVal, bool value);
    
    /**
     * @brief Set return value as integer
     * @param retVal Return value handle
     * @param value  Integer value
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET setReturnInt(MCPReturnValue_t retVal, int value);
    
    /**
     * @brief Set return value as string
     * @param retVal Return value handle
     * @param value  String value
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET setReturnString(MCPReturnValue_t retVal, const char *value);
    
    /**
     * @brief Set return value as JSON object
     * @param retVal Return value handle
     * @param json   cJSON object pointer
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET setReturnJson(MCPReturnValue_t retVal, void *json);
    
    /**
     * @brief Set return value as image
     * @param retVal   Return value handle
     * @param mimeType Image MIME type
     * @param data     Image data
     * @param len      Image data length
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET setReturnImage(MCPReturnValue_t retVal, MCPImageMime_t mimeType,
                        const uint8_t *data, uint32_t len);
    
    /**
     * @brief Get string property from property list
     * @param properties Property list handle
     * @param name       Property name
     * @param defaultVal Default value if not found
     * @return Property value or default
     */
    const char* getPropertyString(MCPPropertyList_t properties, 
                                   const char *name, 
                                   const char *defaultVal = nullptr);
    
    /**
     * @brief Get integer property from property list
     * @param properties Property list handle
     * @param name       Property name
     * @param defaultVal Default value if not found
     * @return Property value or default
     */
    int getPropertyInt(MCPPropertyList_t properties, 
                       const char *name, 
                       int defaultVal = 0);
    
    /**
     * @brief Get boolean property from property list
     * @param properties Property list handle
     * @param name       Property name
     * @param defaultVal Default value if not found
     * @return Property value or default
     */
    bool getPropertyBool(MCPPropertyList_t properties, 
                         const char *name, 
                         bool defaultVal = false);

private:
    bool _initialized;
    bool _available;
};

/***********************************************************
********************Example Usage***************************
***********************************************************/

/**
 * @example Complete MCP Tool Registration Example
 * @code
 * #include <Arduino.h>
 * #include "TuyaAI.h"
 * #include "cJSON.h"
 * 
 * extern "C" {
 * #include "tal_log.h"
 * }
 * 
 * //============================================
 * // Callback Functions
 * //============================================
 * 
 * // Get device information (no properties needed)
 * static int onGetDeviceInfo(MCPPropertyList_t props, MCPReturnValue_t ret, void *data) {
 *     cJSON *json = cJSON_CreateObject();
 *     cJSON_AddStringToObject(json, "model", "MyDevice");
 *     cJSON_AddStringToObject(json, "version", "1.0.0");
 *     TuyaAI.MCP.setReturnJson(ret, json);
 *     return 0;
 * }
 * 
 * // Set device volume (with integer property)
 * static int onSetVolume(MCPPropertyList_t props, MCPReturnValue_t ret, void *data) {
 *     int volume = TuyaAI.MCP.getPropertyInt(props, "volume", 50);
 *     PR_DEBUG("Setting volume to %d", volume);
 *     TuyaAI.MCP.setReturnBool(ret, true);
 *     return 0;
 * }
 * 
 * // Set chat mode
 * static int onSetMode(MCPPropertyList_t props, MCPReturnValue_t ret, void *data) {
 *     int mode = TuyaAI.MCP.getPropertyInt(props, "mode", 0);
 *     PR_DEBUG("Setting mode to %d", mode);
 *     TuyaAI.MCP.setReturnBool(ret, true);
 *     return 0;
 * }
 * 
 * //============================================
 * // MCP Initialization
 * //============================================
 * bool appMCPInit(void) {
 *     // Initialize MCP server
 *     if (!TuyaAI.MCP.begin("My Device", "1.0")) {
 *         PR_ERR("MCP init failed");
 *         return false;
 *     }
 *     
 *     // Register tool without properties
 *     TUYA_MCP_TOOL_ADD_SIMPLE("device_info_get",
 *                              "Get device information",
 *                              onGetDeviceInfo,
 *                              nullptr);
 *     
 *     // Register tool with properties
 *     TuyaMCPPropDef volumeProps[] = {
 *         TuyaMCPPropIntRange("volume", "Volume level (0-100)", 0, 100)
 *     };
 *     TUYA_MCP_TOOL_REGISTER("device_volume_set",
 *                            "Set device volume",
 *                            onSetVolume,
 *                            nullptr,
 *                            volumeProps, 1);
 *     
 *     // Register tool with multiple properties
 *     TuyaMCPPropDef modeProps[] = {
 *         TuyaMCPPropIntRange("mode", "Chat mode (0-3)", 0, 3)
 *     };
 *     TUYA_MCP_TOOL_REGISTER("device_mode_set",
 *                            "Set chat mode",
 *                            onSetMode,
 *                            nullptr,
 *                            modeProps, 1);
 *     
 *     PR_INFO("MCP tools registered");
 *     return true;
 * }
 * @endcode
 */

#endif /* __TUYA_MCP_H_ */
