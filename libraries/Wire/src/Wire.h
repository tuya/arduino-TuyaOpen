#pragma once

#include <Arduino.h>
#include "api/HardwareI2C.h"

#ifndef WIRE_BUFFER_SIZE
#define WIRE_BUFFER_SIZE 128
#endif

class TwoWire : public HardwareI2C {
protected:
  uint8_t _isBegin = 0;

  uint8_t _i2cPort = -1;
  int _clkHz;
  pin_size_t _sda = SDA;
  pin_size_t _scl = SCL;

  size_t _bufferSize;
  uint8_t *_rxBuffer;
  size_t _rxIndex;
  size_t _rxLength;

  uint8_t *_txBuffer;
  size_t _txLength;
  uint16_t _txAddress;

  uint32_t _timeOutMillis;
public:
  TwoWire(uint8_t port);
  ~TwoWire();

  void begin() override;
  void begin(uint8_t address) override;
  void end() override;

  // Select IO pins to use.  Call before ::begin()
  bool setSDA(pin_size_t sda);
  bool setSCL(pin_size_t scl);
  void setClock(uint32_t freq) override;

  void beginTransmission(uint8_t address) override;
  uint8_t endTransmission(bool stopBit) override;
  uint8_t endTransmission(void) override;

  // Not support slave mode
  size_t requestFrom(uint8_t address, size_t len, bool stopBit) override;
  size_t requestFrom(uint8_t address, size_t len) override;

  void onReceive(void(*)(int)) override;
  void onRequest(void(*)(void)) override;

  size_t write(uint8_t) override;
  size_t write(const uint8_t *, size_t) override;
  int available() override;
  int read() override;
  int peek() override;
  void flush() override;

  size_t setBufferSize(size_t bSize);
  void freeWireBuffer();
};

extern TwoWire Wire;
