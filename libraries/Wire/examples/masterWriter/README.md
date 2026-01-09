# I2C Master Writer Example

## Overview

This example demonstrates how to use the Wire library as an I2C master to send data to an I2C slave device. It shows basic I2C communication by transmitting incrementing byte values.

## Features

- I2C master mode operation
- Data transmission to slave device
- Automatic byte increment
- Simple periodic communication

## Hardware Requirements

- Tuya-supported development board
- I2C slave device at address 0x08
- Pull-up resistors on SDA and SCL lines (typically 4.7kΩ)

## I2C Pin Configuration

Default I2C pins (check your board specification):
- **SDA (Data)**: Usually defined by board
- **SCL (Clock)**: Usually defined by board

## How It Works

1. **Initialization**: `Wire.begin()` initializes I2C master mode
2. **Transmission**:
   - Start transmission to device #8 (address 0x08)
   - Send one byte value
   - End transmission
3. **Increment**: Value increases by 1 each transmission
4. **Repeat**: Send new value every 500ms

## Code Explanation

### Setup
```cpp
void setup() {
  Serial.begin(115200);
  Wire.begin();  // Join I2C bus as master (no address needed)
}
```

### Loop
```cpp
Wire.beginTransmission(8);  // Start transmission to device #8
Wire.write(x);              // Send one byte
Wire.endTransmission();     // Stop transmitting
```

## I2C Communication Flow

```
Master                    Slave (Address: 8)
  |                              |
  |------ Start Condition ------>|
  |------ Slave Address (8) ---->|
  |<--------- ACK --------------|
  |------ Data Byte (x) -------->|
  |<--------- ACK --------------|
  |------ Stop Condition ------->|
  |                              |
```

## Usage

1. Connect I2C slave device to I2C pins
2. Ensure pull-up resistors are in place
3. Set slave device address to 0x08 (or modify code)
4. Flash firmware to device
5. Open Serial Monitor at 115200 baud
6. Observe "Sending: X" messages

## Example Output

```
Sending: 0
Sending: 1
Sending: 2
Sending: 3
...
Sending: 255
Sending: 0
Sending: 1
...
```

## Wire Library Methods

### Master Mode

```cpp
Wire.begin();                        // Initialize as master
Wire.beginTransmission(address);     // Start transmission
Wire.write(data);                    // Write data byte
Wire.write(buffer, length);          // Write buffer
Wire.endTransmission();              // End transmission
Wire.requestFrom(address, quantity); // Request data from slave
Wire.available();                    // Check bytes available
Wire.read();                         // Read one byte
```

## Wiring Example

```
Master Device         I2C Slave (Address: 8)
-----------           ---------------------
SDA ----------+-------- SDA
              |
           4.7kΩ (Pull-up to 3.3V)
              |
SCL ----------+-------- SCL
              |
           4.7kΩ (Pull-up to 3.3V)
              |
GND -------------------- GND
3.3V ------------------- VCC
```

## I2C Addresses

- Valid range: 0x08 to 0x77 (8 to 119 decimal)
- Reserved addresses:
  - 0x00 to 0x07: Reserved
  - 0x78 to 0x7F: Reserved

## Common I2C Devices

- Sensors: Temperature, pressure, accelerometer, etc.
- EEPROMs: Memory storage
- RTCs: Real-time clocks
- DACs/ADCs: Analog conversion
- LCD displays with I2C backpack
- GPIO expanders

## Troubleshooting

### No Communication
- Check wiring connections
- Verify pull-up resistors are present
- Confirm slave device address
- Check power supply to slave

### Data Errors
- Reduce I2C clock speed if wiring is long
- Check signal integrity with oscilloscope
- Ensure proper ground connection
- Verify voltage levels match (3.3V or 5V)

### Clock Stretching
- Some slaves use clock stretching
- Ensure your master supports it
- May need to adjust timeout settings

## Advanced Features

### Multiple Slaves
```cpp
Wire.beginTransmission(8);
Wire.write(data1);
Wire.endTransmission();

Wire.beginTransmission(9);
Wire.write(data2);
Wire.endTransmission();
```

### Reading from Slave
```cpp
Wire.requestFrom(8, 2);  // Request 2 bytes from device #8
while (Wire.available()) {
  char c = Wire.read();
  Serial.print(c);
}
```

### Clock Speed
```cpp
Wire.setClock(100000);  // 100kHz (standard mode)
Wire.setClock(400000);  // 400kHz (fast mode)
```

## Performance Considerations

- **Standard Mode**: 100 kHz (default)
- **Fast Mode**: 400 kHz
- **Wire Length**: Keep short for reliable communication
- **Pull-up Resistors**: 4.7kΩ typical, adjust based on capacitance

## Dependencies

- Wire library (included)

## Notes

- Master mode doesn't require address parameter in begin()
- Always use endTransmission() to complete transmission
- Check return value of endTransmission() for errors:
  - 0: Success
  - 1: Data too long
  - 2: NACK on address
  - 3: NACK on data
  - 4: Other error

## Related Examples

- Master Reader: Receiving data from slave
- Slave Sender/Receiver: Implementing I2C slave
