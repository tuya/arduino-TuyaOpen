# 07_AI_PictureInput

Let the AI look at a picture and answer questions about it. The picture comes from the camera; the answer arrives over the normal AI text stream.

> This example only supports the TUYA_T5AI platform with a camera attached. Like every AI component example, the device must be provisioned and connected to the Tuya cloud — picture understanding runs in the cloud, not on the device.

## Two ways to send a picture

| Way | Call | Stored on device | Can carry a question |
| --- | --- | --- | --- |
| Direct recognition | `TuyaAI.Picture.recognize(data, len)` | No | No, the cloud is asked to describe it |
| Album attachment | `TuyaAI.Picture.attachFromAlbum(name, question)` + `sendAttachments()` | Yes | Yes, one question per picture |

The album route lets several pictures be queued (up to `AI_PICTURE_INPUT_MAX_NUM`, 3 by default) and released together, so the AI sees them as one request.

## Vendor build requirement

```
CONFIG_ENABLE_COMP_AI_PICTURE=y   # album + picture input/output
CONFIG_ENABLE_COMP_AI_VIDEO=y     # camera capture
```

Both are already set in this repository's T5 vendor configuration.

## Device Authorization

Replace the placeholders at the top of the sketch with the credentials of your own device:

```cpp
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define TUYA_PRODUCT_ID     "9inb01mvjqh5zhhr"
```

## Flashing Procedure

1. Open `07_AI_PictureInput.ino` in Arduino IDE.
2. Fill in the credentials above.
3. Select the **T5** board (`tuya_open:tuya_open:t5`) and the serial port.
4. Click **Upload**.

## Device Connection and Interaction

1. On first boot the device enters pairing mode and plays the network-configuration prompt (`TUYA_EVENT_BIND_START`).
2. Pair it in the Tuya app. Once MQTT connects the serial monitor prints `Device MQTT Connected!` and the sketch starts accepting commands.
3. In the serial monitor (115200 baud):

| Key | Action |
| --- | --- |
| `r` | Capture a frame and send it straight to the AI for recognition |
| `a` | Capture a frame, save it into the album, attach the question, and send |

Typical output:

```
Device MQTT Connected!
Saved as ai_picture_0001.jpg, asking: What do you see in this picture?
[Picture sent, waiting for the answer]
[AI]: A desk with a keyboard and a coffee mug.
```

Before provisioning finishes the sketch answers:

```
Device is not connected yet, please finish provisioning first.
```

## API

| Method | Signature | Description |
| --- | --- | --- |
| `recognize` | `OPERATE_RET recognize(const uint8_t *data, uint32_t len)` | Send a JPEG straight to the AI; nothing is stored |
| `attachFromAlbum` | `OPERATE_RET attachFromAlbum(const char *filename, const char *text = nullptr)` | Queue an album picture, optionally with a question |
| `detachFromAlbum` | `OPERATE_RET detachFromAlbum(const char *filename)` | Remove a queued attachment |
| `sendAttachments` | `OPERATE_RET sendAttachments()` | Send every queued attachment to the AI |
| `saveToAlbum` | `OPERATE_RET saveToAlbum(const uint8_t *data, uint32_t len, const char *hint, char *outName, uint32_t outNameSize)` | Store a JPEG in the album and return its filename |

### Events

| Event | Payload | Meaning |
| --- | --- | --- |
| `AI_USER_EVT_SEND_PICTURE_END` | none | The picture finished uploading; the answer follows |
| `AI_USER_EVT_TEXT_STREAM_START` / `_DATA` / `_STOP` | text bytes | The AI's answer, streamed |

## Notes

- Every frame from `TuyaAI.Video.getJpegFrame()` must be released with `freeJpegFrame()`, which this example does on both paths.
- `saveToAlbum()` copies the final filename into the caller's buffer; size it at least `AI_PICTURE_NAME_MAX_LEN + 1` (65 bytes).
- Storing a picture is a local operation, but sending one to the AI is not — that is why the sketch gates both commands on the MQTT connection.
- The live camera preview is opened with `AI_UI_DISP_CAMERA_OPEN` and closed with `AI_UI_DISP_CAMERA_CLOSE`; receiving AI-generated pictures is covered by `08_AI_PictureGenerate`.
