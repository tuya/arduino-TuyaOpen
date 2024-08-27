#include "SPI.h"

extern "C" {
#include <string.h>
#include "tal_log.h"
}

using namespace arduino;

SPIClassTuyaOpen::SPIClassTuyaOpen()
{
  TUYA_SPI_BASE_CFG_T cfg;

  cfg.role = TUYA_SPI_ROLE_MASTER;
  cfg.mode = TUYA_SPI_MODE0;
  cfg.type = TUYA_SPI_SOFT_TYPE;
  cfg.databits = TUYA_SPI_DATA_BIT8;
  cfg.bitorder = TUYA_SPI_ORDER_LSB2MSB;
  cfg.freq_hz = SPI_DEFAULT_CLOCK;
  cfg.spi_dma_flags = 1;

  _spiConfigInitAlwaysInline(TUYA_SPI_NUM_0, cfg);
}

SPIClassTuyaOpen::SPIClassTuyaOpen(TUYA_SPI_NUM_E port)
{
  TUYA_SPI_BASE_CFG_T cfg;

  cfg.role = TUYA_SPI_ROLE_MASTER;
  cfg.mode = TUYA_SPI_MODE0;
  cfg.type = TUYA_SPI_SOFT_TYPE;
  cfg.databits = TUYA_SPI_DATA_BIT8;
  cfg.bitorder = TUYA_SPI_ORDER_LSB2MSB;
  cfg.freq_hz = SPI_DEFAULT_CLOCK;
  cfg.spi_dma_flags = 1;

  _spiConfigInitAlwaysInline(port, cfg);
}

SPIClassTuyaOpen::SPIClassTuyaOpen(TUYA_SPI_NUM_E port, TUYA_SPI_BASE_CFG_T cfg)
{
  _spiConfigInitAlwaysInline(port, cfg);
}

SPIClassTuyaOpen::~SPIClassTuyaOpen()
{
  end();
}

void SPIClassTuyaOpen::begin()
{
  begin(true);
}

void SPIClassTuyaOpen::begin(bool autoCS)
{
  _autoCS = autoCS;

  if (_autoCS) {
    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, LOW);
  }
}

void SPIClassTuyaOpen::end()
{
  endTransaction();
}

void SPIClassTuyaOpen::beginTransaction(SPISettings settings)
{
  OPERATE_RET rt = OPRT_OK;

  _cfg.freq_hz = settings.getClockFreq();

  switch ( settings.getDataMode() ) {
    case SPI_MODE0 : {
      _cfg.mode = TUYA_SPI_MODE0;
    } break;
    case SPI_MODE1 : {
      _cfg.mode = TUYA_SPI_MODE1;
    } break;
    case SPI_MODE2 : {
      _cfg.mode = TUYA_SPI_MODE2;
    } break;
    case SPI_MODE3 : {
      _cfg.mode = TUYA_SPI_MODE3;
    } break;
    default : break;
  }

  _cfg.bitorder = (settings.getBitOrder() == LSBFIRST) ? (TUYA_SPI_ORDER_LSB2MSB) : (TUYA_SPI_ORDER_MSB2LSB);

  rt = tkl_spi_init(_port, &_cfg);
  if (OPRT_OK != rt) {
    PR_ERR("spi[%d] init fail, %d", _port, rt);
  }

  if (_autoCS) {
    digitalWrite(_csPin, HIGH);
  }
}

void SPIClassTuyaOpen::endTransaction(void)
{
  tkl_spi_deinit(_port);

  if (_autoCS) {
    digitalWrite(_csPin, LOW);
  }
}

uint8_t SPIClassTuyaOpen::transfer(uint8_t data)
{
  OPERATE_RET rt = OPRT_OK;

  uint8_t out = 0;

  rt = tkl_spi_transfer(_port, static_cast<void*>(&data), static_cast<void*>(&out), 1);
  if (OPRT_OK == rt) {
    return 0;
  }
  return out;
}

uint16_t SPIClassTuyaOpen::transfer16(uint16_t data)
{
  OPERATE_RET rt = OPRT_OK;

  uint16_t out = 0;

  rt = tkl_spi_transfer(_port, static_cast<void*>(&data), static_cast<void*>(&out), 2);
  if (OPRT_OK == rt) {
    return 0;
  }
  return out;
}

void SPIClassTuyaOpen::transfer(void *buf, size_t count)
{
  tkl_spi_transfer(_port, buf, NULL, count);
}

SPIClassTuyaOpen SPI(TUYA_SPI_NUM_0);
