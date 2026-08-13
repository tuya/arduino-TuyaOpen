# 07_AI_PictureInput

让 AI 看图并回答关于图片的问题。图片来自摄像头,回答通过常规的 AI 文本流返回。

> 本示例仅支持带摄像头的 TUYA_T5AI 平台。与所有 AI 组件示例一样,设备必须完成配网并连接涂鸦云 —— 图片理解在云端完成,不在设备本地。

## 两种送图方式

| 方式 | 调用 | 是否存本地 | 能否带问题 |
| --- | --- | --- | --- |
| 直接识别 | `TuyaAI.Picture.recognize(data, len)` | 否 | 否,由云端直接描述 |
| 相册附件 | `TuyaAI.Picture.attachFromAlbum(name, question)` + `sendAttachments()` | 是 | 是,每张图配一个问题 |

相册方式可以排队多张图片(上限 `AI_PICTURE_INPUT_MAX_NUM`,默认 3 张)后一起发送,AI 会作为一次请求整体处理。

## Vendor 构建前置条件

```
CONFIG_ENABLE_COMP_AI_PICTURE=y   # 相册 + 图片输入/输出
CONFIG_ENABLE_COMP_AI_VIDEO=y     # 摄像头采集
```

本仓库的 T5 vendor 配置已经全部设置。

## 设备授权

把示例顶部的占位符替换成你自己设备的授权信息:

```cpp
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define TUYA_PRODUCT_ID     "9inb01mvjqh5zhhr"
```

## 烧录步骤

1. 用 Arduino IDE 打开 `07_AI_PictureInput.ino`。
2. 填入上面的授权信息。
3. 选择 **T5** 开发板(`tuya_open:tuya_open:t5`)与对应串口。
4. 点击 **上传**。

## 设备连接与交互

1. 首次上电设备进入配网模式,并播放配网提示音(`TUYA_EVENT_BIND_START`)。
2. 在涂鸦 App 中完成配网。MQTT 连接成功后串口打印 `Device MQTT Connected!`,示例开始接受指令。
3. 在串口监视器(115200)中:

| 按键 | 动作 |
| --- | --- |
| `r` | 抓一帧,直接送 AI 识别 |
| `a` | 抓一帧,存入相册,附上问题后发送 |

典型输出:

```
Device MQTT Connected!
Saved as ai_picture_0001.jpg, asking: What do you see in this picture?
[Picture sent, waiting for the answer]
[AI]: A desk with a keyboard and a coffee mug.
```

配网完成前,示例会提示:

```
Device is not connected yet, please finish provisioning first.
```

## API

| 方法 | 签名 | 说明 |
| --- | --- | --- |
| `recognize` | `OPERATE_RET recognize(const uint8_t *data, uint32_t len)` | 把 JPEG 直接送给 AI,不落盘 |
| `attachFromAlbum` | `OPERATE_RET attachFromAlbum(const char *filename, const char *text = nullptr)` | 把相册图片排入队列,可附带问题 |
| `detachFromAlbum` | `OPERATE_RET detachFromAlbum(const char *filename)` | 从队列中移除某张图片 |
| `sendAttachments` | `OPERATE_RET sendAttachments()` | 把队列中的图片一并发给 AI |
| `saveToAlbum` | `OPERATE_RET saveToAlbum(const uint8_t *data, uint32_t len, const char *hint, char *outName, uint32_t outNameSize)` | 把 JPEG 存进相册并返回文件名 |

### 事件

| 事件 | 负载 | 含义 |
| --- | --- | --- |
| `AI_USER_EVT_SEND_PICTURE_END` | 无 | 图片上传完成,随后是回答 |
| `AI_USER_EVT_TEXT_STREAM_START` / `_DATA` / `_STOP` | 文本字节 | AI 的流式回答 |

## 注意事项

- `TuyaAI.Video.getJpegFrame()` 拿到的每一帧都必须用 `freeJpegFrame()` 释放,本示例的两条路径都做了。
- `saveToAlbum()` 会把最终文件名拷回调用方缓冲区,缓冲区至少要 `AI_PICTURE_NAME_MAX_LEN + 1`(65 字节)。
- 存图是本地操作,送 AI 不是 —— 所以示例把两个指令都用 MQTT 连接状态做了门控。
- 摄像头实时预览由 `AI_UI_DISP_CAMERA_OPEN` 打开、`AI_UI_DISP_CAMERA_CLOSE` 关闭;接收 AI 生成的图片见 `08_AI_PictureGenerate`。
