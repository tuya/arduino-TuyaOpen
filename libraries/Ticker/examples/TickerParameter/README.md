# Ticker with Parameters Example

## Overview

This example demonstrates how to pass parameters to Ticker callback functions. It shows how to use the same callback function with different parameters for multiple timers.

## Features

- Pass parameters to timer callbacks
- Multiple timers with same callback function
- Different timing intervals
- Parameter differentiation in callbacks
- Non-blocking operation

## Hardware Requirements

- Tuya-supported development board
- Serial Monitor for output

## How It Works

1. **Timer 1**: Calls callback every 1 second with parameter value 1
2. **Timer 2**: Calls callback every 5 seconds with parameter value 5
3. **Callback**: Prints the parameter value to differentiate which timer triggered

## Ticker with Parameters

### Syntax
```cpp
ticker.attach(interval, callback, parameter);
```

- `interval`: Time in seconds between callbacks
- `callback`: Function to call
- `parameter`: Pointer to parameter data

## Code Explanation

### Setup
```cpp
int arg_1s = 1;
Ticker ticker_1s;

int arg_5s = 5;
Ticker ticker_5s;

ticker_1s.attach(1, ticker_callback, &arg_1s);
ticker_5s.attach(5, ticker_callback, &arg_5s);
```

### Callback Function
```cpp
void ticker_callback(int *arg)
{
  Serial.print("arg: ");
  Serial.println(*arg);
}
```

## Usage

1. Flash firmware to device
2. Open Serial Monitor at 115200 baud
3. Observe output:
   - "arg: 1" appears every 1 second
   - "arg: 5" appears every 5 seconds

## Example Output

```
arg: 1
arg: 1
arg: 1
arg: 1
arg: 5
arg: 1
arg: 1
...
```

## Parameter Types

You can pass different types of parameters:

### Integer
```cpp
int value = 42;
ticker.attach(1.0, callback, &value);

void callback(int *arg) {
  Serial.println(*arg);
}
```

### Float
```cpp
float value = 3.14;
ticker.attach(1.0, callback, &value);

void callback(float *arg) {
  Serial.println(*arg);
}
```

### Structure
```cpp
struct Data {
  int id;
  float value;
};

Data data = {1, 23.5};
ticker.attach(1.0, callback, &data);

void callback(Data *arg) {
  Serial.println(arg->id);
  Serial.println(arg->value);
}
```

### String/Array
```cpp
char message[] = "Hello";
ticker.attach(1.0, callback, message);

void callback(char *arg) {
  Serial.println(arg);
}
```

## Advanced Example

### Multiple Sensors
```cpp
struct SensorConfig {
  int pin;
  const char* name;
  float threshold;
};

SensorConfig temp = {A0, "Temperature", 25.0};
SensorConfig hum = {A1, "Humidity", 60.0};

Ticker tempTicker;
Ticker humTicker;

void setup() {
  tempTicker.attach(2, readSensor, &temp);
  humTicker.attach(5, readSensor, &hum);
}

void readSensor(SensorConfig *config) {
  int value = analogRead(config->pin);
  Serial.print(config->name);
  Serial.print(": ");
  Serial.println(value);
}
```

## Use Cases

- Multiple sensors with different reading intervals
- Different LED blink patterns with same callback
- State machines with timer-triggered states
- Multi-channel data logging
- Scheduled tasks with task-specific parameters

## Important Considerations

### Parameter Lifetime
- Parameters must remain valid during timer operation
- Don't use local variables that go out of scope
- Use global or static variables

✅ **Good**:
```cpp
int globalValue = 10;
ticker.attach(1, callback, &globalValue);
```

❌ **Bad**:
```cpp
void setup() {
  int localValue = 10;  // Will be destroyed after setup()
  ticker.attach(1, callback, &localValue);  // WRONG!
}
```

### Thread Safety
- Be careful with shared data
- Use volatile for variables accessed in callbacks
- Consider disabling interrupts for critical sections

### Memory Management
- Static allocation is preferred
- Avoid dynamic allocation in callbacks
- Ensure parameter memory isn't freed while timer is active

## Callback Best Practices

✅ **Do**:
- Keep callbacks short and fast
- Use parameters for configuration
- Set flags for main loop processing
- Use volatile for shared variables

❌ **Don't**:
- Perform lengthy operations
- Use Serial.print() excessively
- Call delay()
- Allocate memory dynamically

## Troubleshooting

- **Wrong values**: Check parameter pointers
- **Crashes**: Verify parameter lifetime
- **No output**: Ensure Serial is initialized
- **Incorrect timing**: Check if callbacks are too slow

## Dependencies

- Ticker library (included)

## Related Examples

- Blinker: Basic Ticker usage without parameters
- Multiple timers: Managing several independent timers

## Notes

- Parameters are passed by pointer (reference)
- The callback function must match the parameter type
- Multiple timers can share the same callback with different parameters
- This pattern enables code reuse and reduces duplication
