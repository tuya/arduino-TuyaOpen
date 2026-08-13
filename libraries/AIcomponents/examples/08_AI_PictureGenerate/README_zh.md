# 08_AI_PictureGenerate

让 AI 画图,把结果存进设备相册并显示到屏幕上。

> 本示例仅支持带屏幕的 TUYA_T5AI 平台。与所有 AI 组件示例一样,设备必须完成配网并连接涂鸦云 —— 图片生成在云端完成。

## 流程

```
setOutputSize(w, h)          告诉云端按什么分辨率渲染
        |
以文本发送提示词              "Draw a cat wearing a hat"
        |
图片写入相册                  由 AI picture 组件完成
        |
AI_USER_EVT_GENERATE_PICTURE 带回相册文件名
        |
AI_UI_DISP_ALBUM_OPEN        显示到屏幕
```

从涂鸦 App 推图到设备时,`AI_USER_EVT_GET_PICTURE_FROM_APP` 走的是同一套流程。

## Vendor 构建前置条件

```
CONFIG_ENABLE_COMP_AI_PICTURE=y
```

以及一块屏幕 —— 本仓库的 T5 vendor 配置默认启用 T5AI 开发板的 3.5 寸 LCD。若要从云端文件存储下载生成的图片,还需 `CONFIG_ENABLE_COMP_AI_PICTURE_HOSTING_DLD=y`;未开启时 `beginOutputDownload()` 返回 `OPRT_NOT_SUPPORTED`。

## 设备授权

把示例顶部的占位符替换成你自己设备的授权信息:

```cpp
#define TUYA_DEVICE_UUID    "uuidxxxxxxxxxxxxxxxx"
#define TUYA_DEVICE_AUTHKEY "keyxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define TUYA_PRODUCT_ID     "9inb01mvjqh5zhhr"
```

## 烧录步骤

1. 用 Arduino IDE 打开 `08_AI_PictureGenerate.ino`。
2. 填入上面的授权信息。
3. 选择 **T5** 开发板(`tuya_open:tuya_open:t5`)与对应串口。
4. 点击 **上传**。

## 设备连接与交互

1. 首次上电设备进入配网模式,并播放配网提示音。
2. 在涂鸦 App 中完成配网;串口打印 `Device MQTT Connected!` 表示就绪。
3. 在串口监视器(115200)中:

| 输入 | 动作 |
| --- | --- |
| 任意文本 + 回车 | 作为画图提示词发送 |
| `v` | 在屏幕上打开相册缩略图墙 |

典型输出:

```
Device MQTT Connected!

[Prompt]: Draw a cat wearing a hat
[AI]: Here is the picture you asked for.

[Picture stored as]: ai_picture_0002.jpg
```

事件到达后,生成的图片会立即显示在屏幕上。

## API

| 方法 | 签名 | 说明 |
| --- | --- | --- |
| `setOutputSize` | `OPERATE_RET setOutputSize(uint16_t width, uint16_t height)` | 指定 AI 渲染分辨率 |
| `beginOutputDownload` | `OPERATE_RET beginOutputDownload(uint16_t width, uint16_t height)` | 开启从云端文件存储下载生成图片 |
| `albumName` | `const char *albumName()` | 图片所在相册的名称 |

屏幕显示通过通用 UI 透传完成:

```cpp
TuyaAI.UI.displayMessage(AI_UI_DISP_ALBUM_OPEN,      nullptr, 0);  // 查看当前图片
TuyaAI.UI.displayMessage(AI_UI_DISP_ALBUM_VIEW_ALL,  nullptr, 0);  // 缩略图墙
TuyaAI.UI.displayMessage(AI_UI_DISP_ALBUM_VIEW_NEXT, nullptr, 0);  // 下一张
TuyaAI.UI.displayMessage(AI_UI_DISP_ALBUM_VIEW_PREV, nullptr, 0);  // 上一张
```

### 事件

| 事件 | 负载 | 含义 |
| --- | --- | --- |
| `AI_USER_EVT_GENERATE_PICTURE` | 相册文件名 | AI 画完并已存盘 |
| `AI_USER_EVT_GET_PICTURE_FROM_APP` | 相册文件名 | 从涂鸦 App 收到图片 |
| `AI_USER_EVT_TEXT_STREAM_START` / `_DATA` / `_STOP` | 文本字节 | AI 附带的文字回复 |

## 注意事项

- 两个图片事件的负载都是以 NUL 结尾的相册文件名,Arduino 层按 `data` + 实际 `len` 传出。
- 相册最多保留 `COMP_AI_PICTURE_ALBUM_MAX_IMAGE_CNT` 张(默认 10),超出后旧图会被丢弃。
- 默认输出尺寸 320x480,与上游的 `COMP_AI_PICTURE_DEF_OUTPIUT_WIDTH/HEIGHT` 一致。
- 相册页面属于微信风格 UI,任何 `AI_UI_DISP_ALBUM_*` 消息之前必须先执行 `TuyaAI.UI.begin(BOT_UI_WECHAT)`。
- 把生成的图片打印出来,可与 Peripherals 库中的 `Printer` 示例组合使用。
