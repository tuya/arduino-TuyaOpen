# SPI Demo Example

## Overview

This example demonstrates basic SPI (Serial Peripheral Interface) communication on Tuya boards. It shows how to initialize SPI and transfer data using the SPI library.

## Features

- SPI bus initialization
- Data transfer using SPI
- Configurable SPI settings (speed, bit order, mode)
- Transaction-based communication

## Hardware Requirements

- Tuya-supported board
- SPI peripheral device (optional for testing)

## Pin Configuration

- **SCK (Clock)**: GPIO 14
- **MOSI (Master Out Slave In)**: GPIO 16
- **MISO (Master In Slave Out)**: GPIO 17
- **CS (Chip Select)**: GPIO 15

**Note**: Pin assignments may vary by board. Check your board's pinout.

## SPI Settings

```cpp
SPISettings(8000000, MSBFIRST, SPI_MODE0)
```

- **Clock Speed**: 8 MHz (recommended range: 1-8 MHz for T2)
- **Bit Order**: MSBFIRST (Most Significant Bit First) - T2 only supports MSB first
- **SPI Mode**: SPI_MODE0 (CPOL=0, CPHA=0)

## SPI Modes

| Mode | CPOL | CPHA | Description |
|------|------|------|-------------|
| MODE0 | 0 | 0 | Clock idle low, sample on leading edge |
| MODE1 | 0 | 1 | Clock idle low, sample on trailing edge |
| MODE2 | 1 | 0 | Clock idle high, sample on leading edge |
| MODE3 | 1 | 1 | Clock idle high, sample on trailing edge |

## How It Works

1. **Initialization**: `SPI.begin()` initializes the SPI bus
2. **Transaction Start**: `beginTransaction()` sets SPI parameters
3. **Data Transfer**: `transfer()` sends data to SPI device
4. **Transaction End**: `endTransaction()` releases SPI bus
5. **Repeat**: Transfers data every 3 seconds

## Usage

1. Connect your SPI device to the specified pins
2. Flash the firmware to your device
3. The device will continuously send "Hello Arduino." via SPI
4. Use a logic analyzer or oscilloscope to verify SPI signals

## Code Example

```cpp
SPI.begin();

SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
SPI.transfer(data, length);
SPI.endTransaction();
```

## Transfer Methods

### Single Byte Transfer
```cpp
uint8_t response = SPI.transfer(data);
```

### Buffer Transfer
```cpp
SPI.transfer(buffer, bufferSize);
```

### Bidirectional Transfer
```cpp
SPI.transfer(txBuffer, rxBuffer, length);
```

## Wiring Example

```
Board (T2)         SPI Device
---------          ----------
GPIO 14 (SCK)  --> SCK
GPIO 16 (MOSI) --> MOSI/SDI
GPIO 17 (MISO) <-- MISO/SDO
GPIO 15 (CS)   --> CS/SS
GND            --> GND
3.3V           --> VCC
```

## Important Notes

- **T2 Clock Range**: Recommended 1-8 MHz
- **T2 Bit Order**: Only MSBFIRST is supported
- **Voltage Level**: Ensure SPI device is 3.3V compatible
- **CS (Chip Select)**: Must be controlled manually if needed
- **Pull-ups**: Some devices may require pull-up resistors

## Common SPI Devices

- SD Cards
- LCD/OLED displays
- Sensors (accelerometers, gyroscopes)
- ADC/DAC converters
- Flash memory chips
- Radio modules

## Troubleshooting

- **No communication**: Check wiring and pin assignments
- **Garbled data**: Verify clock speed and SPI mode
- **Intermittent errors**: Check CS timing and signal integrity
- **Device not responding**: Verify power supply and ground connections

## Performance Tips

- Use DMA for large data transfers (if supported)
- Minimize transaction overhead for high-speed communication
- Keep wire lengths short for signal integrity
- Use appropriate clock speed for your device

## Dependencies

- SPI library (included)

## Learn More

- Study SPI protocol specifications
- Use logic analyzer to debug SPI communication
- Check your SPI device datasheet for proper settings
- Experiment with different clock speeds and modes
