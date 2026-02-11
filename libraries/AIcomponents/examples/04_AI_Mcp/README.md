# AI MCP Tool Invocation

This example demonstrates how to use the **MCP (Model Context Protocol)** feature in the Arduino-TuyaOpen framework to register custom tools for AI Agent invocation. When users converse with the AI via voice or text, the AI can autonomously determine and call device-side registered tools to achieve intelligent hardware control.

> This example only supports the TUYA_T5AI platform. It is recommended to use the [T5AI-Board Development Kit](https://tuyaopen.ai/en/docs/hardware-specific/t5-ai-board/overview-t5-ai-board) for development.

## How It Works

1. The device registers tools with the cloud via the MCP protocol (including tool name, description, and parameter definitions)
2. The user sends a request to the AI via voice or text (e.g., "Set the volume to 50")
3. The AI large language model automatically matches and invokes the corresponding device-side tool based on user intent
4. The device executes the tool callback function, completes the operation, and returns the result to the AI
5. The AI responds to the user with the execution result in natural language

## Flashing Procedure

0. Make sure you have completed the development environment setup described in [Quick Start](Quick_start.md).

1. Connect the T5AI development board to your computer, open Arduino IDE, select the `TUYA_T5AI` board, and choose the correct upload port.

> Note: T5AI series development boards provide dual serial port communication. When connected to a computer, two serial port numbers will be detected. UART0 is used for firmware flashing — please select the correct upload port in Arduino IDE.

2. In Arduino IDE, click `File` -> `Examples` -> `AI components` -> `04_AI_Mcp` to open the example code.

3. Replace the authorization code and product PID in the example file with your own credentials.
    - [What is an authorization code](https://tuyaopen.ai/en/docs/quick-start#tuyaopen-authorization-code)
    - [How to obtain an authorization code](https://tuyaopen.ai/en/docs/quick-start#obtaining-tuyaopen-authorization-code)

```cpp
// Device authorization code (replace with your own)
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

```

4. Click the upload button in the top-left corner of Arduino IDE to flash the code. The following messages in the terminal indicate a successful flash.

```bash
[INFO]: Write flash success
[INFO]: CRC check success
[INFO]: Reboot done
[INFO]: Flash write success.
```

## Device Connection and Interaction

### Device Connection

After flashing the firmware, the development board needs to connect to the network and register with the cloud.

Device provisioning guide: [Device Network Configuration](https://tuyaopen.ai/en/docs/quick-start/device-network-configuration)

> **Quick Re-provisioning**: Press the reset button 3 times in quick succession, and the device will clear its saved network configuration and re-enter the provisioning state.

### Device Interaction

After successful provisioning, you can converse with the AI via voice or serial text input. Below are the MCP tools registered in this example and how to trigger them:

| Voice/Text Command | Triggered MCP Tool | Function |
| --- | --- | --- |
| "Set the volume to 50" | `device_audio_volume_set` | Set device volume (0–100) |

> Users can express their needs in natural language, and the AI will automatically understand and invoke the corresponding tool.

## Example Code Explanation

### MCP Initialization

MCP tools must be registered **after the MQTT connection is established**. This example subscribes to the `EVENT_MQTT_CONNECTED` event to ensure correct registration timing:

```cpp
// In setup(), subscribe to the MQTT connection event with a one-time callback
tal_event_subscribe(EVENT_MQTT_CONNECTED, "mcpRegisterTool", initMCPTools, SUBSCRIBE_TYPE_ONETIME);
```

The MCP initialization and tool registration are completed in the callback function `initMCPTools`:

```cpp
static int initMCPTools(void* data) {
    // 1. Initialize MCP service
    if (OPRT_OK != TuyaAI.MCP.begin()) {
        PR_ERR("MCP init failed");
        return -1;
    }

    // 2. Define tool parameters
    TuyaMCPPropDef volumeProps[] = {
        TuyaMCPPropIntRange("volume", "The volume level to set (0-100).", 0, 100)
    };

    // 3. Register tool with parameters
    TUYA_MCP_TOOL_REGISTER(
        "device_audio_volume_set",                    // Tool name
        "Sets the device's volume level.\n"           // Tool description (for AI to understand purpose)
        "Parameters:\n"
        "- volume (int): The volume level (0-100).\n"
        "Response:\n"
        "- Returns true if set successfully.",
        onSetVolume,                                  // Callback function
        nullptr,                                      // User data
        volumeProps, 1                                // Parameter definitions and count
    );
    return OPRT_OK;
}
```

### MCP Tool Callback

When the AI invokes a registered tool, the corresponding callback function is executed:

```cpp
static int onSetVolume(MCPPropertyList_t props, MCPReturnValue_t ret, void *userData) {
    // Get the volume value from the parameter list (default 50)
    int volume = TuyaAI.MCP.getPropertyInt(props, "volume", 50);
    // Execute volume setting
    TuyaAI.Audio.setVolume(volume);
    // Return execution result
    TuyaAI.MCP.setReturnBool(ret, true);
    return 0;
}
```

### MCP Registration Methods

MCP provides two registration methods:

**1. Registration with parameters** (using the `TUYA_MCP_TOOL_REGISTER` macro):

```cpp
TuyaMCPPropDef volumeProps[] = {
    TuyaMCPPropIntRange("volume", "The volume level (0-100).", 0, 100)
};
TUYA_MCP_TOOL_REGISTER("tool_name", "description", callback, userData, volumeProps, 1);
```

**2. Simple registration** (parameterless tool, using the `TUYA_MCP_TOOL_ADD_SIMPLE` macro):

```cpp
TUYA_MCP_TOOL_ADD_SIMPLE("tool_name", "description", callback, userData);
```

### Parameter Type Macros

| Macro | Description |
| --- | --- |
| `TuyaMCPPropStr(name, desc)` | String type parameter |
| `TuyaMCPPropIntRange(name, desc, min, max)` | Integer type parameter (with range constraint) |
| `TuyaMCPPropIntDefRange(name, desc, def, min, max)` | Integer type parameter (with default value and range) |

### Common Methods in Callback Functions

| Method | Description |
| --- | --- |
| `TuyaAI.MCP.getPropertyInt(props, name, def)` | Get integer parameter value |
| `TuyaAI.MCP.getPropertyString(props, name, def)` | Get string parameter value |
| `TuyaAI.MCP.getPropertyBool(props, name, def)` | Get boolean parameter value |
| `TuyaAI.MCP.setReturnBool(ret, value)` | Return a boolean value |
| `TuyaAI.MCP.setReturnJson(ret, json)` | Return a JSON object |
| `TuyaAI.MCP.setReturnImage(ret, mime, data, len)` | Return image data |

### AI Event Callback

This example handles the following AI events:

| Event | Description |
| --- | --- |
| `AI_USER_EVT_ASR_OK` | ASR speech recognition successful; displays recognized text on the serial port |
| `AI_USER_EVT_TEXT_STREAM_START` | AI text response begins (includes MCP tool invocation results) |
| `AI_USER_EVT_TEXT_STREAM_DATA` | AI text response data |
| `AI_USER_EVT_TEXT_STREAM_STOP` | AI text response ends |

> After the AI invokes an MCP tool, the tool's return result is passed to the large language model as context, and the model then replies to the user in natural language via TEXT_STREAM events.

## Related Documentation

- [What is MCP](https://tuyaopen.ai/en/docs/cloud/tuya-cloud/ai-agent/mcp-management)
- [TuyaOpen Official Website](https://tuyaopen.ai)
- [GitHub Repository](https://github.com/tuya/TuyaOpen)
