# Printer

Drive an ESC/POS thermal printer from Arduino: print text, print a 1-bit bitmap, feed paper, and react to driver events such as running out of paper.

> This example only supports the TUYA_T5AI platform. The printer is a board peripheral, so it needs no network connection and no Tuya cloud account.

## Hardware

| Item | Value |
| --- | --- |
| Printer | DP-48A ESC/POS thermal printer |
| Interface | UART0 |
| Baud rate | 9600 bps (board default) |

The board registers the printer device inside `board_register_hardware()`, so the sketch only has to open it by name.

## Vendor build requirement

The printer driver is compiled into the vendor SDK only when the board option is enabled:

```
CONFIG_TUYA_T5AI_BOARD_PRINTER_DP48=y
```

which in turn selects `ENABLE_PRINTER` and `ENABLE_PRINTER_DP48`. This repository's T5 vendor configuration (`tools/ci/config/t5/app_default.config`) already sets it. Without it, `Printer.open()` returns `OPRT_NOT_FOUND`.

## Flashing Procedure

1. Open `Printer.ino` in Arduino IDE.
2. Select the **T5** board (`tuya_open:tuya_open:t5`) and the serial port of your device.
3. Click **Upload**.

Or from the command line:

```bash
arduino-cli compile --fqbn tuya_open:tuya_open:t5 Printer.ino
arduino-cli upload -p <port> --fqbn tuya_open:tuya_open:t5 Printer.ino
```

## Running the Example

On boot the sketch opens the printer, reports its width, and prints one job. The serial monitor (115200 baud) shows:

```
============ Tuya Thermal Printer ==============
Compile time:        <build date>
Printer ready, 384 dots per line
Print job done
```

The paper carries the heading, two text lines, a separator, a small bitmap, and then advances so the printed part clears the tear bar.

If the printer is missing or the driver was not built in:

```
Printer open failed, rt=-6
Is CONFIG_TUYA_T5AI_BOARD_PRINTER_DP48 enabled in the vendor build?
```

## API

| Method | Signature | Description |
| --- | --- | --- |
| `open` | `OPERATE_RET open(const char *name = nullptr)` | Open the printer registered by the board; `nullptr` uses the board's `PRINTER_NAME` |
| `close` | `void close()` | Close the printer and release its resources |
| `isOpen` | `bool isOpen()` | Whether the printer is currently open |
| `setEventCallback` | `void setEventCallback(PrinterEventCallback_t cb, void *arg = nullptr)` | Install a driver-event callback; call before `open()` |
| `beginJob` | `OPERATE_RET beginJob()` | Start a print job |
| `endJob` | `OPERATE_RET endJob()` | Finish the current job |
| `print` | `OPERATE_RET print(const char *text)` | Print a NUL-terminated string |
| `write` | `OPERATE_RET write(const uint8_t *data, uint32_t len)` | Send raw bytes (ESC/POS commands or pre-encoded data) |
| `printBitmap` | `OPERATE_RET printBitmap(uint16_t x, uint16_t width, uint16_t height, const uint8_t *data)` | Print a 1-bit-per-pixel bitmap, `(width + 7) / 8` bytes per row |
| `feed` | `OPERATE_RET feed(uint32_t lines)` | Advance the paper by `lines` dot rows |
| `getInfo` | `OPERATE_RET getInfo(TDL_PRINTER_DEV_INFO_T *info)` | Read `dots_per_line` / `bytes_per_line` |
| `dotsPerLine` | `uint32_t dotsPerLine()` | Printable width in dots, or 0 when unavailable |

### Events

| Event | Meaning |
| --- | --- |
| `TDL_PRINTER_EVENT_PAPER_OUT` | The printer ran out of paper |
| `TDL_PRINTER_EVENT_PAPER_IN` | Paper was reloaded |
| `TDL_PRINTER_EVENT_OVERHEATED` | The print head is too hot; printing pauses |
| `TDL_PRINTER_EVENT_TEMP_NORMAL` | The print head cooled down |
| `TDL_PRINTER_EVENT_ERROR` | The driver reported an error |

## Notes

- A print job must be bracketed by `beginJob()` and `endJob()`.
- Bitmap rows are packed most-significant-bit first, `(width + 7) / 8` bytes per row. Content past the right edge is clipped by the driver.
- `PRINTER_TEAR_FEED_LINES` in the vendor configuration makes `endJob()` feed automatically. It defaults to 0, which is why this example calls `feed()` explicitly.
- Printing AI-generated pictures is a natural companion to `08_AI_PictureGenerate`, where the upstream UI raises `AI_UI_DISP_PRINT_RESULT` after a print attempt.
