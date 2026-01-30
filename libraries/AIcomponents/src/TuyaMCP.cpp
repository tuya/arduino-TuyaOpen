/**
 * @file TuyaMCP.cpp
 * @author Tuya Inc.
 * @brief TuyaMCPClass implementation - MCP server management
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */
#include "TuyaAI.h"
#include <string.h>
#include "tuya_error_code.h"

#if defined(ENABLE_COMP_AI_MCP) && (ENABLE_COMP_AI_MCP == 1)
#include "ai_mcp/include/ai_mcp.h"
#include "ai_mcp/include/ai_mcp_server.h"
#endif

/***********************************************************
***********************TuyaMCPClass Implementation**********
***********************************************************/

TuyaMCPClass::TuyaMCPClass() {
    _initialized = false;
#if defined(ENABLE_COMP_AI_MCP) && (ENABLE_COMP_AI_MCP == 1)
    _available = true;
#else
    _available = false;
#endif
}

TuyaMCPClass::~TuyaMCPClass() {
    if (_initialized) {
        end();
    }
}

OPERATE_RET TuyaMCPClass::begin() {
#if defined(ENABLE_COMP_AI_MCP) && (ENABLE_COMP_AI_MCP == 1)
    if (_initialized) {
        return OPRT_OK;
    }
    
    OPERATE_RET rt = ai_mcp_server_init("Tuya MCP Server", "1.0");
    // OPERATE_RET rt = ai_mcp_init();
    if (rt == OPRT_OK) {
        _initialized = true;
    }
    return rt;
#else
    return OPRT_NOT_SUPPORTED;
#endif
}

void TuyaMCPClass::end() {
#if defined(ENABLE_COMP_AI_MCP) && (ENABLE_COMP_AI_MCP == 1)
    if (!_initialized) {
        return;
    }
    
    ai_mcp_server_destroy();
    _initialized = false;
#endif
}

bool TuyaMCPClass::isInitialized() {
    return _initialized;
}

bool TuyaMCPClass::isAvailable() {
    return _available;
}

OPERATE_RET TuyaMCPClass::registerTool(const char *name, const char *description,
                                 MCPToolCallback_t callback) {
    return registerTool(name, description, callback, nullptr);
}

OPERATE_RET TuyaMCPClass::registerTool(const char *name, const char *description,
                                 MCPToolCallback_t callback, void *userData) {
#if defined(ENABLE_COMP_AI_MCP) && (ENABLE_COMP_AI_MCP == 1)
    if (!_initialized || name == nullptr || description == nullptr || callback == nullptr) {
        return OPRT_INVALID_PARM;
    }
    
    MCP_TOOL_CALLBACK internalCb = (MCP_TOOL_CALLBACK)callback;
    return ai_mcp_tool_register(name, description, internalCb, userData, MCP_PROP_END);
#else
    return OPRT_NOT_SUPPORTED;
#endif
}

OPERATE_RET TuyaMCPClass::setReturnBool(MCPReturnValue_t retVal, bool value) {
#if defined(ENABLE_COMP_AI_MCP) && (ENABLE_COMP_AI_MCP == 1)
    ai_mcp_return_value_set_bool((MCP_RETURN_VALUE_T *)retVal, value ? TRUE : FALSE);
    return OPRT_OK;
#else
    return OPRT_NOT_SUPPORTED;
#endif
}

OPERATE_RET TuyaMCPClass::setReturnInt(MCPReturnValue_t retVal, int value) {
#if defined(ENABLE_COMP_AI_MCP) && (ENABLE_COMP_AI_MCP == 1)
    ai_mcp_return_value_set_int((MCP_RETURN_VALUE_T *)retVal, value);
    return OPRT_OK;
#else
    return OPRT_NOT_SUPPORTED;
#endif
}

OPERATE_RET TuyaMCPClass::setReturnString(MCPReturnValue_t retVal, const char *value) {
#if defined(ENABLE_COMP_AI_MCP) && (ENABLE_COMP_AI_MCP == 1)
    return ai_mcp_return_value_set_str((MCP_RETURN_VALUE_T *)retVal, value);
#else
    return OPRT_NOT_SUPPORTED;
#endif
}

OPERATE_RET TuyaMCPClass::setReturnJson(MCPReturnValue_t retVal, void *json) {
#if defined(ENABLE_COMP_AI_MCP) && (ENABLE_COMP_AI_MCP == 1)
    ai_mcp_return_value_set_json((MCP_RETURN_VALUE_T *)retVal, (cJSON *)json);
    return OPRT_OK;
#else
    return OPRT_NOT_SUPPORTED;
#endif
}

OPERATE_RET TuyaMCPClass::setReturnImage(MCPReturnValue_t retVal, MCPImageMime_t mimeType,
                                   const uint8_t *data, uint32_t len) {
#if defined(ENABLE_COMP_AI_MCP) && (ENABLE_COMP_AI_MCP == 1)
    const char *internalMime;
    switch (mimeType) {
        case MCP_MIME_JPEG: internalMime = MCP_IMAGE_MIME_TYPE_JPEG; break;
        case MCP_MIME_PNG:  internalMime = MCP_IMAGE_MIME_TYPE_PNG; break;
        case MCP_MIME_GIF:  internalMime = "image/gif"; break;
        case MCP_MIME_WEBP: internalMime = "image/webp"; break;
        default:            internalMime = MCP_IMAGE_MIME_TYPE_JPEG; break;
    }
    return ai_mcp_return_value_set_image((MCP_RETURN_VALUE_T *)retVal, internalMime, (uint8_t *)data, len);
#else
    return OPRT_NOT_SUPPORTED;
#endif
}

const char* TuyaMCPClass::getPropertyString(MCPPropertyList_t properties, 
                                             const char *name, 
                                             const char *defaultVal) {
#if defined(ENABLE_COMP_AI_MCP) && (ENABLE_COMP_AI_MCP == 1)
    MCP_PROPERTY_LIST_T *props = (MCP_PROPERTY_LIST_T *)properties;
    if (props == nullptr || name == nullptr) {
        return defaultVal;
    }
    
    for (int i = 0; i < props->count; i++) {
        MCP_PROPERTY_T *prop = props->properties[i];
        if (strcmp(prop->name, name) == 0 && prop->type == MCP_PROPERTY_TYPE_STRING) {
            return prop->default_val.str_val;
        }
    }
    return defaultVal;
#else
    return defaultVal;
#endif
}

int TuyaMCPClass::getPropertyInt(MCPPropertyList_t properties, 
                                  const char *name, 
                                  int defaultVal) {
#if defined(ENABLE_COMP_AI_MCP) && (ENABLE_COMP_AI_MCP == 1)
    MCP_PROPERTY_LIST_T *props = (MCP_PROPERTY_LIST_T *)properties;
    if (props == nullptr || name == nullptr) {
        return defaultVal;
    }
    
    for (int i = 0; i < props->count; i++) {
        MCP_PROPERTY_T *prop = props->properties[i];
        if (strcmp(prop->name, name) == 0 && prop->type == MCP_PROPERTY_TYPE_INTEGER) {
            return prop->default_val.int_val;
        }
    }
    return defaultVal;
#else
    return defaultVal;
#endif
}

bool TuyaMCPClass::getPropertyBool(MCPPropertyList_t properties, 
                                    const char *name, 
                                    bool defaultVal) {
#if defined(ENABLE_COMP_AI_MCP) && (ENABLE_COMP_AI_MCP == 1)
    MCP_PROPERTY_LIST_T *props = (MCP_PROPERTY_LIST_T *)properties;
    if (props == nullptr || name == nullptr) {
        return defaultVal;
    }
    
    for (int i = 0; i < props->count; i++) {
        MCP_PROPERTY_T *prop = props->properties[i];
        if (strcmp(prop->name, name) == 0 && prop->type == MCP_PROPERTY_TYPE_BOOLEAN) {
            return prop->default_val.bool_val != 0;
        }
    }
    return defaultVal;
#else
    return defaultVal;
#endif
}

/***********************************************************
***********************Global Functions*********************
***********************************************************/

#if defined(ENABLE_COMP_AI_MCP) && (ENABLE_COMP_AI_MCP == 1)

/**
 * @brief Register an MCP tool with properties (C++ wrapper)
 * 
 * This is a C++ wrapper that internally calls the same functions as AI_MCP_TOOL_ADD macro.
 * It converts TuyaMCPPropDef array to MCP_PROPERTY_DEF_T and uses ai_mcp_property_create_from_def()
 * exactly like ai_mcp_tool_register() does.
 */
int TuyaMCPToolRegister(const char *name, const char *description,
                        MCPToolCallback_t callback, void *userData,
                        const TuyaMCPPropDef *props, int propCount) {
    if (name == nullptr || description == nullptr || callback == nullptr) {
        return OPRT_INVALID_PARM;
    }
    
    // Create tool (same as ai_mcp_tool_register does)
    MCP_TOOL_T *tool = ai_mcp_tool_create(name, description, 
                                           (MCP_TOOL_CALLBACK)callback, userData);
    if (!tool) {
        return OPRT_MALLOC_FAILED;
    }
    
    // Add properties using ai_mcp_property_create_from_def (same as ai_mcp_tool_register)
    OPERATE_RET rt = OPRT_OK;
    if (props != nullptr && propCount > 0) {
        for (int i = 0; i < propCount; i++) {
            // Convert TuyaMCPPropDef to MCP_PROPERTY_DEF_T
            MCP_PROPERTY_DEF_T propDef = {0};
            propDef.name = props[i].name;
            propDef.type = props[i].type;
            propDef.description = props[i].description;
            propDef.has_default = props[i].has_default;
            propDef.has_range = props[i].has_range;
            propDef.min_val = props[i].min_val;
            propDef.max_val = props[i].max_val;
            
            // Set default value based on type
            if (props[i].has_default) {
                switch (props[i].type) {
                    case MCP_PROPERTY_TYPE_STRING:
                        propDef.default_val.str_val = props[i].default_str;
                        break;
                    case MCP_PROPERTY_TYPE_INTEGER:
                        propDef.default_val.int_val = props[i].default_int;
                        break;
                    case MCP_PROPERTY_TYPE_BOOLEAN:
                        propDef.default_val.bool_val = props[i].default_bool;
                        break;
                    default:
                        break;
                }
            }
            
            // Create property from definition (same as ai_mcp_tool_register)
            MCP_PROPERTY_T *prop = ai_mcp_property_create_from_def(&propDef);
            if (prop) {
                rt = ai_mcp_tool_add_property(tool, prop);
                if (rt != OPRT_OK) {
                    ai_mcp_property_destroy(prop);
                    ai_mcp_tool_destroy(tool);
                    return rt;
                }
            }
        }
    }
    
    // Register tool with server (same as ai_mcp_tool_register)
    rt = ai_mcp_server_add_tool(tool);
    if (rt != OPRT_OK) {
        ai_mcp_tool_destroy(tool);
        return rt;
    }
    
    return OPRT_OK;
}

#endif // ENABLE_COMP_AI_MCP
