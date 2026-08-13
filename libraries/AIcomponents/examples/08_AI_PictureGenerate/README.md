# 08_AI_PictureGenerate

Ask the AI to draw something, then store the result in the on-device album and show it on the display.

> This example only supports the TUYA_T5AI platform with a display. Like every AI component example, the device must be provisioned and connected to the Tuya cloud — picture generation runs in the cloud.

## Flow

```
setOutputSize(w, h)          tell the cloud what resolution to render at
        |
send the prompt as text      "Draw a cat wearing a hat"
        |
picture stored in the album  handled by the AI picture component
        |
AI_USER_EVT_GENERATE_PICTURE carries the album filename
        |
AI_UI_DISP_ALBUM_OPEN        show it on the display
```

`AI_USER_EVT_GET_PICTURE_FROM_APP` follows the same shape when a picture is pushed from the Tuya app instead of being generated.

## Vendor build requirement

```
CONFIG_ENABLE_COMP_AI_PICTURE=y
```

plus a display — the T5AI board's 3.5" LCD is configured by default in this repository's T5 vendor configuration. Downloading generated pictures from cloud file storage additionally needs `CONFIG_ENABLE_COMP_AI_PICTURE_HOSTING_DLD=y`; without it `beginOutputDownload()` returns `OPRT_NOT_SUPPORTED`.

## Device Authorization

Replace the placeholders at the top of the sketch with the credentials of your own device:

```cpp
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define TUYA_PRODUCT_ID     "9inb01mvjqh5zhhr"
```

## Flashing Procedure

1. Open `08_AI_PictureGenerate.ino` in Arduino IDE.
2. Fill in the credentials above.
3. Select the **T5** board (`tuya_open:tuya_open:t5`) and the serial port.
4. Click **Upload**.

## Device Connection and Interaction

1. On first boot the device enters pairing mode and plays the network-configuration prompt.
2. Pair it in the Tuya app; the serial monitor prints `Device MQTT Connected!` when it is ready.
3. In the serial monitor (115200 baud):

| Input | Action |
| --- | --- |
| any text + Enter | Send it as a drawing prompt |
| `v` | Open the album thumbnail wall on the display |

Typical output:

```
Device MQTT Connected!

[Prompt]: Draw a cat wearing a hat
[AI]: Here is the picture you asked for.

[Picture stored as]: ai_picture_0002.jpg
```

The generated picture appears on the display as soon as the event arrives.

## API

| Method | Signature | Description |
| --- | --- | --- |
| `setOutputSize` | `OPERATE_RET setOutputSize(uint16_t width, uint16_t height)` | Resolution the AI should render at |
| `beginOutputDownload` | `OPERATE_RET beginOutputDownload(uint16_t width, uint16_t height)` | Enable downloading generated pictures from cloud file storage |
| `albumName` | `const char *albumName()` | Name of the album the pictures land in |

Display control goes through the generic UI passthrough:

```cpp
TuyaAI.UI.displayMessage(AI_UI_DISP_ALBUM_OPEN,      nullptr, 0);  // view the current picture
TuyaAI.UI.displayMessage(AI_UI_DISP_ALBUM_VIEW_ALL,  nullptr, 0);  // thumbnail wall
TuyaAI.UI.displayMessage(AI_UI_DISP_ALBUM_VIEW_NEXT, nullptr, 0);  // next picture
TuyaAI.UI.displayMessage(AI_UI_DISP_ALBUM_VIEW_PREV, nullptr, 0);  // previous picture
```

### Events

| Event | Payload | Meaning |
| --- | --- | --- |
| `AI_USER_EVT_GENERATE_PICTURE` | album filename | The AI finished drawing and stored the result |
| `AI_USER_EVT_GET_PICTURE_FROM_APP` | album filename | A picture arrived from the Tuya app |
| `AI_USER_EVT_TEXT_STREAM_START` / `_DATA` / `_STOP` | text bytes | The AI's accompanying reply |

## Notes

- Both picture events carry a NUL-terminated album filename; the Arduino layer passes it as `data` with `len` set to its length.
- The album keeps at most `COMP_AI_PICTURE_ALBUM_MAX_IMAGE_CNT` pictures (10 by default); older ones are dropped.
- The default output size is 320x480, matching `COMP_AI_PICTURE_DEF_OUTPIUT_WIDTH/HEIGHT` upstream.
- The album pages belong to the WeChat-style UI, so `TuyaAI.UI.begin(BOT_UI_WECHAT)` must run before any `AI_UI_DISP_ALBUM_*` message.
- Printing the generated picture pairs naturally with the `Printer` example in the Peripherals library.
