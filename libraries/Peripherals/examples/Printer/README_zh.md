# Printer

在 Arduino 中驱动 ESC/POS 热敏打印机:打印文字、打印 1bit 位图、走纸,并响应缺纸等驱动事件。

> 本示例仅支持 TUYA_T5AI 平台。打印机属于板级外设,不需要联网,也不需要涂鸦云账号。

## 硬件

| 项目 | 取值 |
| --- | --- |
| 打印机 | DP-48A ESC/POS 热敏打印机 |
| 接口 | UART0 |
| 波特率 | 9600 bps(板级默认) |

板级 `board_register_hardware()` 会自动注册打印机设备,示例里只需按名字打开它。

## Vendor 构建前置条件

打印机驱动只有在开启板级选项时才会编进 vendor SDK:

```
CONFIG_TUYA_T5AI_BOARD_PRINTER_DP48=y
```

它会连带 `select ENABLE_PRINTER` 和 `ENABLE_PRINTER_DP48`。本仓库的 T5 vendor 配置(`tools/ci/config/t5/app_default.config`)已经设置。未开启时 `Printer.open()` 返回 `OPRT_NOT_FOUND`。

## 烧录步骤

1. 用 Arduino IDE 打开 `Printer.ino`。
2. 选择 **T5** 开发板(`tuya_open:tuya_open:t5`)与设备对应的串口。
3. 点击 **上传**。

命令行方式:

```bash
arduino-cli compile --fqbn tuya_open:tuya_open:t5 Printer.ino
arduino-cli upload -p <port> --fqbn tuya_open:tuya_open:t5 Printer.ino
```

## 运行示例

上电后示例会打开打印机、报告打印宽度并打印一份内容。串口监视器(115200)输出:

```
============ Tuya Thermal Printer ==============
Compile time:        <编译日期>
Printer ready, 384 dots per line
Print job done
```

纸上依次是标题、两行文字、分隔线、一个小位图,最后走纸让已打印部分越过撕纸口。

如果打印机不存在或驱动未编入:

```
Printer open failed, rt=-6
Is CONFIG_TUYA_T5AI_BOARD_PRINTER_DP48 enabled in the vendor build?
```

## API

| 方法 | 签名 | 说明 |
| --- | --- | --- |
| `open` | `OPERATE_RET open(const char *name = nullptr)` | 打开板级注册的打印机;传 `nullptr` 使用板级 `PRINTER_NAME` |
| `close` | `void close()` | 关闭打印机并释放资源 |
| `isOpen` | `bool isOpen()` | 打印机当前是否已打开 |
| `setEventCallback` | `void setEventCallback(PrinterEventCallback_t cb, void *arg = nullptr)` | 注册驱动事件回调,需在 `open()` 之前调用 |
| `beginJob` | `OPERATE_RET beginJob()` | 开始一次打印任务 |
| `endJob` | `OPERATE_RET endJob()` | 结束当前打印任务 |
| `print` | `OPERATE_RET print(const char *text)` | 打印以 NUL 结尾的字符串 |
| `write` | `OPERATE_RET write(const uint8_t *data, uint32_t len)` | 发送原始字节(ESC/POS 指令或已编码数据) |
| `printBitmap` | `OPERATE_RET printBitmap(uint16_t x, uint16_t width, uint16_t height, const uint8_t *data)` | 打印 1bit 位图,每行 `(width + 7) / 8` 字节 |
| `feed` | `OPERATE_RET feed(uint32_t lines)` | 走纸 `lines` 个点行 |
| `getInfo` | `OPERATE_RET getInfo(TDL_PRINTER_DEV_INFO_T *info)` | 读取 `dots_per_line` / `bytes_per_line` |
| `dotsPerLine` | `uint32_t dotsPerLine()` | 可打印宽度(点),不可用时返回 0 |

### 事件

| 事件 | 含义 |
| --- | --- |
| `TDL_PRINTER_EVENT_PAPER_OUT` | 打印机缺纸 |
| `TDL_PRINTER_EVENT_PAPER_IN` | 纸张已装回 |
| `TDL_PRINTER_EVENT_OVERHEATED` | 打印头过热,打印暂停 |
| `TDL_PRINTER_EVENT_TEMP_NORMAL` | 打印头温度恢复正常 |
| `TDL_PRINTER_EVENT_ERROR` | 驱动报错 |

## 注意事项

- 一次打印任务必须由 `beginJob()` 和 `endJob()` 包裹。
- 位图按最高位在前打包,每行 `(width + 7) / 8` 字节;超出右边界的内容由驱动裁掉。
- vendor 配置里的 `PRINTER_TEAR_FEED_LINES` 可让 `endJob()` 自动走纸,默认为 0,因此本示例显式调用了 `feed()`。
- 打印 AI 生成的图片是它与 `08_AI_PictureGenerate` 的自然组合 —— 上游 UI 在打印完成后会抛出 `AI_UI_DISP_PRINT_RESULT`。
