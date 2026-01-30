/**
 * @file AppMCP.h
 * @brief MCP (Model Context Protocol) interface for AI ChatBot
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#ifndef __APP_MCP_H__
#define __APP_MCP_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize and register all MCP tools
 * @return OPRT_OK on success, error code on failure
 */
OPERATE_RET appMCPInit(void);

/**
 * @brief Deinitialize MCP
 */
void appMCPDeinit(void);

#ifdef __cplusplus
}
#endif

#endif // __APP_MCP_H__