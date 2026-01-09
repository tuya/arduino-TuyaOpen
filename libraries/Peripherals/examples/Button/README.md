# Button Example

## Overview

This example demonstrates how to use the Button class to handle various button events on Tuya IoT devices. It shows proper button initialization, debouncing, and event handling for multiple buttons.

## Features

- Multiple button support
- Button event detection (press down, press up, single click)
- Configurable debounce time
- Long press detection
- Multi-click detection
- Pull-up/pull-down configuration
- Event callback system

## Hardware Requirements

- Tuya-supported development board
- Two buttons (or modify for one button):
  - Button 0: GPIO 12
  - Button 1: GPIO 13
- Buttons can be wired active-low (with pull-up) or active-high (with pull-down)

## Pin Configuration

- `BUTTON_PIN0`: GPIO 12 - First button
- `BUTTON_PIN1`: GPIO 13 - Second button

## Button Events

The following events can be detected:

- `BUTTON_EVENT_PRESS_DOWN`: Button is pressed
- `BUTTON_EVENT_PRESS_UP`: Button is released
- `BUTTON_EVENT_SINGLE_CLICK`: Single click detected
- `BUTTON_EVENT_DOUBLE_CLICK`: Double click detected (configurable)
- `BUTTON_EVENT_LONG_PRESS_START`: Long press detected
- `BUTTON_EVENT_LONG_PRESS_HOLD`: Held during long press
- ···

## Configuration Parameters

```cpp
ButtonConfig_t cfg;
cfg.debounceTime = 50;          // Debounce time in ms
cfg.longPressTime = 2000;       // Long press threshold in ms
cfg.longPressHoldTime = 500;    // Hold repeat interval in ms
cfg.multiClickCount = 2;        // Number of clicks for multi-click
cfg.multiClickInterval = 500;   // Max interval between clicks in ms
```

## Pin Configuration

```cpp
PinConfig_t pinCfg;
pinCfg.pin = BUTTON_PIN0;                // GPIO pin number
pinCfg.level = TUYA_GPIO_LEVEL_LOW;      // Active level (LOW or HIGH)
pinCfg.pullMode = TUYA_GPIO_PULLUP;      // Pull-up or pull-down
```

## How It Works

1. **Initialization**: Configure button parameters and GPIO pins
2. **Event Registration**: Register callbacks for desired events
3. **Automatic Detection**: Button class handles debouncing and event detection
4. **Callback Execution**: Registered functions are called when events occur

## Usage

1. Connect buttons to specified GPIO pins
2. Flash firmware to device
3. Open Serial Monitor at 115200 baud
4. Press buttons to see events in serial output

## Example Output

```
[Button0] Event: 0  // Press down
Button Down

[Button0] Event: 1  // Press up
Button UP

[Button0] Event: 2  // Single click
Button Single Click
```

## Customization

### Adding More Events

```cpp
Button0.setEventCallback(BUTTON_EVENT_DOUBLE_CLICK, buttonCallback);
Button0.setEventCallback(BUTTON_EVENT_LONG_PRESS_START, buttonCallback);
```

### Active High Configuration

```cpp
pinCfg.level = TUYA_GPIO_LEVEL_HIGH;
pinCfg.pullMode = TUYA_GPIO_PULLDOWN;
```

### Adjusting Timing

```cpp
cfg.debounceTime = 30;        // Faster debounce
cfg.longPressTime = 1000;     // Shorter long press
```

## Event Callback

```cpp
void buttonCallback(char *name, ButtonEvent_t event, void *arg)
{
    // name: Button identifier ("Button0", "Button1")
    // event: Type of button event
    // arg: Optional user data
}
```

## Dependencies

- Button library (Peripherals)
- Log library

## Common Use Cases

- Single button: Power on/off
- Double click: Mode switching
- Long press: Factory reset
- Multiple buttons: Navigation, control
- Hold: Continuous adjustment

## Notes

- Debouncing eliminates false triggers from mechanical bounce
- Long press time should be > debounce time
- Multi-click interval determines click speed tolerance
- Each button can have different configurations
- Button class uses non-blocking detection

## Troubleshooting

- If events don't trigger: Check wiring and pull configuration
- If multiple events trigger: Increase debounce time
- If long press too sensitive: Increase longPressTime
- Check serial output for debug information

## Best Practices

- Use appropriate debounce time for your buttons (20-100ms typical)
- Set long press time based on user experience (1-3 seconds)
- Register only needed events to save resources
- Test button response with actual hardware
