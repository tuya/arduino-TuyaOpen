# Ticker Blinker Example

## Overview

This example demonstrates how to use the Ticker library to create periodic and one-time timer callbacks. It shows LED blinking at different rates using non-blocking timers.

## Features

- Periodic timer callbacks
- One-time timer callbacks
- Non-blocking operation
- Multiple independent timers
- Dynamic timer reconfiguration
- LED control demonstration

## Hardware Requirements

- Tuya-supported development board
- Built-in LED (LED_BUILTIN)

## How It Works

1. **Blinker Timer**: Toggles LED every 0.1 seconds (10 Hz)
2. **Changer Timer**: After 10 seconds, changes blink rate to 0.5 seconds (2 Hz)
3. **Non-blocking**: All timing happens in the background

## Ticker Methods

### Periodic Callbacks

```cpp
ticker.attach(seconds, callback);
```
Calls `callback` every `seconds` seconds.

### One-Time Callback

```cpp
ticker.once(seconds, callback);
```
Calls `callback` once after `seconds` seconds.

### Stop Timer

```cpp
ticker.detach();
```
Stops the timer.

## Code Explanation

### Initial Setup
```cpp
blinker.attach(0.1, blinkCallback);  // Blink every 0.1s
changer.once(10, changeCallback);     // Change after 10s
```

### Callback Functions

**blinkCallback()**: Toggles LED state
```cpp
void blinkCallback() {
  ledStatus = !ledStatus;
  digitalWrite(LED_BUILTIN, ledStatus);
}
```

**changeCallback()**: Changes blink rate
```cpp
void changeCallback() {
  blinkerPace = 0.5;               // New rate: 0.5s
  blinker.detach();                 // Stop old timer
  blinker.attach(0.5, blinkCallback); // Start with new rate
}
```

## Usage

1. Flash firmware to your device
2. LED will blink rapidly (10 times per second)
3. After 10 seconds, LED will blink slowly (2 times per second)
4. The change happens automatically

## Timing Characteristics

| Phase | Rate | Frequency | Duration |
|-------|------|-----------|----------|
| Initial | 0.1s | 10 Hz | 10 seconds |
| After change | 0.5s | 2 Hz | Indefinite |

## Common Use Cases

- LED status indicators
- Periodic sensor readings
- Heartbeat signals
- Timeout detection
- Auto-save functionality
- Periodic network requests

## Important Notes

- **Non-blocking**: Does not use delay()
- **Background execution**: Callbacks run in interrupt context
- **Keep callbacks short**: Don't perform long operations in callbacks
- **Multiple timers**: Can create multiple Ticker instances
- **Precision**: Timing is generally accurate but not real-time guaranteed

## Callback Best Practices

✅ **Good**:
- Toggle GPIO
- Set flags
- Update counters
- Simple state changes

❌ **Avoid**:
- Serial.print() (use sparingly)
- Delay functions
- Long calculations
- Memory allocation
- Blocking operations

## Advanced Examples

### Multiple LEDs
```cpp
Ticker led1Ticker;
Ticker led2Ticker;

led1Ticker.attach(0.5, toggleLed1);
led2Ticker.attach(0.3, toggleLed2);
```

### Timeout Detection
```cpp
Ticker timeout;

void startOperation() {
  timeout.once(5.0, handleTimeout);
  // Perform operation
}

void operationComplete() {
  timeout.detach();  // Cancel timeout
}
```

### Periodic Data Collection
```cpp
Ticker dataTicker;

void setup() {
  dataTicker.attach(60, collectData);  // Every minute
}

void collectData() {
  // Read sensors, log data, etc.
}
```

## Troubleshooting

- **LED doesn't blink**: Check LED_BUILTIN definition and wiring
- **Timing not accurate**: Callbacks may be affected by other interrupts
- **System instability**: Check if callbacks are too long or complex
- **Memory issues**: Avoid dynamic allocation in callbacks

## Dependencies

- Ticker library (included)

## Notes

- Ticker uses hardware timers internally
- Timing resolution depends on system tick rate
- Callbacks execute in interrupt context
- Be mindful of concurrent access to shared variables
- Use volatile keyword for variables shared between ISR and main code

## Related Examples

- TickerParameter: Shows how to pass parameters to callbacks
- See Arduino Timer documentation for more timer options
