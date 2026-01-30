/**
 * @file TuyaAI.h
 * @author Tuya Inc.
 * @brief TuyaAI Arduino C++ wrapper for AI components
 *
 * This header provides a nested class architecture for Tuya AI functionality:
 * - TuyaAIClass: Main AI controller with nested sub-classes
 *   - TuyaUIClass: UI display management
 *   - TuyaAudioClass: Audio input/output management
 *   - TuyaMCPClass: MCP (Model Context Protocol) integration
 *
 * Features:
 * - Chat mode management (Hold, OneShot, Wakeup, Free)
 * - Event callback system for user customization
 * - Multi-language support
 * - Clean abstraction without exposing internal C headers
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 *
 */
#ifndef __TUYA_AI_H_
#define __TUYA_AI_H_

#include "TuyaIoT.h"

#include "tuya_cloud_types.h"
#include "tal_api.h"
#include "board_com_api.h"

// Include all sub-component headers
#include "TuyaAI_Types.h"
#include "TuyaUI.h"
#include "TuyaAudio.h"
#include "TuyaMCP.h"
#include "TuyaSkill.h"

/***********************************************************
***********************class definition*********************
***********************************************************/

/**
 * @class TuyaAIClass
 * @brief Main class for Tuya AI functionality with nested sub-classes
 * 
 * This class provides a comprehensive interface for interacting with the
 * Tuya AI system. It contains nested classes for UI, Audio, and MCP.
 * 
 * @note Button handling is NOT included - use Button class separately
 * 
 * @endcode
 */
class TuyaAIClass {
public:
    /**
     * @brief Constructor
     */
    TuyaAIClass();
    
    /**
     * @brief Destructor
     */
    ~TuyaAIClass();

    //==========================================================================
    // Nested Class Instances
    //==========================================================================
    
    TuyaUIClass    UI;      /**< UI display management */
    TuyaAudioClass Audio;   /**< Audio input/output management */
    TuyaMCPClass   MCP;     /**< MCP (Model Context Protocol) management */
    TuyaSkillClass Skill;   /**< Skill data parsing */

    //==========================================================================
    // Initialization & Configuration
    //==========================================================================
    
    /**
     * @brief Initialize AI component with configuration
     * @param config AI configuration structure
     * @return OPRT_OK on success, error code on failure
     * @note This only initializes core AI, call UI.begin() and MCP.begin() separately
     */
    OPERATE_RET begin(AIConfig_t &config);
    
    /**
     * @brief Initialize AI component with default settings
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET begin();
    
    /**
     * @brief Deinitialize AI component
     */
    void end();
    
    /**
     * @brief Check if AI component is initialized
     * @return true if initialized
     */
    bool isInitialized();

    //==========================================================================
    // Input Methods
    //==========================================================================
    
    /**
     * @brief Send text input to AI
     * @param text Text content to send
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET snedText(const char *text);
    
    /**
     * @brief Send text input from buffer
     * @param buffer Text buffer
     * @param len    Buffer length
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET snedText(uint8_t *buffer, int len);
    
    /**
     * @brief Start voice input recording
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET startVoiceInput();
    
    /**
     * @brief Stop voice input recording
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET stopVoiceInput();
    
    /**
     * @brief Send image to AI for analysis
     * @param data Image data buffer
     * @param len  Image data length
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET sendImage(uint8_t *data, uint32_t len);
    
    /**
     * @brief Send file to AI
     * @param data File data buffer
     * @param len  File data length
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET sendFile(uint8_t *data, uint32_t len);

    //==========================================================================
    // Chat Mode Control
    //==========================================================================
    
    /**
     * @brief Set chat mode
     * @param mode Chat mode to set
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET setChatMode(AIChatMode_t mode);
    
    /**
     * @brief Get current chat mode
     * @return Current chat mode
     */
    AIChatMode_t getChatMode();
    
    /**
     * @brief Switch to next chat mode
     * @return New chat mode
     */
    AIChatMode_t nextChatMode();
    
    /**
     * @brief Get current AI state
     * @return Current AI state
     */
    AIState_t getState();
    
    /**
     * @brief Get state as string
     * @return State string representation
     */
    const char* getStateString();
    
    /**
     * @brief Get mode as string
     * @return Mode string representation
     */
    const char* getModeString();
    
    /**
     * @brief Save current mode configuration
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET loadModeConfig(AIChatMode_t *mode, int *volume);

    /**
     * @brief Load mode configuration
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET saveModeConfig(AIChatMode_t mode, int volume);

    /**
     * @brief Handle mode key event
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET modeKeyHandle(uint8_t event, void *arg);

    /**
     * @brief Interrupt current AI conversation
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET interruptChat();

    //==========================================================================
    // AI Agent Control
    //==========================================================================
    
    /**
     * @brief Switch AI agent role/persona
     * @param roleName Role name to switch to
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET switchRole(const char *roleName);
    
    /**
     * @brief Request cloud alert from AI
     * @param type Alert type
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET requestCloudAlert(AIAlertType_t type);

    //==========================================================================
    // Callback Registration
    //==========================================================================
    
    /**
     * @brief Set event callback
     * @param callback Event callback function
     * @param arg      User argument passed to callback
     */
    void setEventCallback(AIEventCallback_t callback, void *arg = nullptr);
    
    /**
     * @brief Set state change callback
     * @param callback State callback function
     */
    void setStateCallback(AIStateCallback_t callback);
    
    /**
     * @brief Set custom alert callback
     * @param callback Alert callback function
     * @note Return 0 in callback to handle alert yourself
     */
    void setAlertCallback(AIAlertCallback_t callback);

    //==========================================================================
    // Language Configuration
    //==========================================================================
    
    /**
     * @brief Get current language code
     * @return Language code string (e.g., "zh-CN", "en-US")
     */
    const char* getLanguageCode();

    //==========================================================================
    // Convenience Methods (delegate to sub-classes)
    //==========================================================================
    
    /**
     * @brief Set audio volume (convenience, delegates to Audio.setVolume)
     * @param volume Volume value (0-100)
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET setVolume(int volume) { return Audio.setVolume(volume); }
    
    /**
     * @brief Get audio volume (convenience, delegates to Audio.getVolume)
     * @return Current volume (0-100)
     */
    int getVolume() { return Audio.getVolume(); }
    
    /**
     * @brief Play alert (convenience, delegates to Audio.playAlert)
     * @param type Alert type
     * @return OPRT_OK on success, error code on failure
     */
    OPERATE_RET playAlert(AIAlertType_t type) { return Audio.playAlert(type); }

public:
    // Public accessor methods for nested classes
    AIEventCallback_t getEventCallback() { return _eventCallback; }
    AIStateCallback_t getStateCallback() { return _stateCallback; }
    AIAlertCallback_t getAlertCallback() { return _alertCallback; }
    void* getUserArg() { return _userArg; }

private:
    bool _initialized;
    AIChatMode_t _chatMode;
    
    AIEventCallback_t   _eventCallback;
    AIStateCallback_t   _stateCallback;
    AIAlertCallback_t   _alertCallback;
    void               *_userArg;
};

// Global instance
extern TuyaAIClass TuyaAI;

#endif /* __TUYA_AI_H_ */
