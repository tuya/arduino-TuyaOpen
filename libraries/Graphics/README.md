# Graphics Library - Display C++ Wrapper

这个库提供了一个面向对象的 C++ 接口来封装 Tuya Display Layer (TDL) 的 C 语言显示驱动。

## 功能特性

- **设备管理**: 初始化、查找和关闭显示设备
- **显示控制**: 亮度调节、帧缓冲刷新
- **绘图操作**: 
  - 绘制单个像素
  - 填充矩形区域
  - 清屏/全屏填充
- **颜色转换**: RGB888 ↔ RGB565 ↔ 其他像素格式
- **设备信息查询**: 宽度、高度、像素格式

## API 参考

### 初始化和设备管理

```cpp
Display display;  // 创建显示对象

OPERATE_RET begin();           // 初始化显示设备
void end();                     // 关闭显示设备
bool isInitialized() const;     // 检查是否已初始化
```

### 显示控制

```cpp
OPERATE_RET setBrightness(uint8_t brightness);  // 设置亮度 (0-100)
OPERATE_RET flush();                             // 刷新帧缓冲到显示器
```

### 绘图操作

```cpp
// 清除显示（默认黑色）
OPERATE_RET clear(uint32_t color = 0x000000);

// 绘制单个像素
OPERATE_RET drawPixel(uint16_t x, uint16_t y, uint32_t color);

// 填充矩形区域 (x0,y0) 到 (x1,y1)
OPERATE_RET fillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t color);

// 填充整个屏幕
OPERATE_RET fillScreen(uint32_t color);
```

### 颜色转换

```cpp
// 转换颜色格式
uint32_t convertColor(uint32_t color, 
                     TUYA_DISPLAY_PIXEL_FMT_E srcFmt, 
                     TUYA_DISPLAY_PIXEL_FMT_E dstFmt, 
                     uint32_t threshold = 32768);

// RGB565 转换为当前显示格式
uint32_t rgb565ToColor(uint16_t rgb565, uint32_t threshold = 32768);

// RGB888 (8-8-8) 转换为 RGB565 (5-6-5)
uint16_t rgb888ToRgb565(uint8_t r, uint8_t g, uint8_t b);
```

### 信息查询

```cpp
uint16_t getWidth() const;                          // 获取显示宽度
uint16_t getHeight() const;                         // 获取显示高度
TUYA_DISPLAY_PIXEL_FMT_E getPixelFormat() const;   // 获取像素格式
```

## 使用示例

### 基本示例

```cpp
#include "display.h"

Display display;

void setup() {
    // 初始化显示
    if (display.begin() != OPRT_OK) {
        Serial.println("Failed to initialize display");
        return;
    }
    
    // 清除屏幕（黑色）
    display.clear(0x000000);
    display.flush();
}

void loop() {
    // 填充屏幕为红色
    display.fillScreen(0xFF0000);
    display.flush();
    delay(1000);
    
    // 填充屏幕为绿色
    display.fillScreen(0x00FF00);
    display.flush();
    delay(1000);
    
    // 填充屏幕为蓝色
    display.fillScreen(0x0000FF);
    display.flush();
    delay(1000);
}
```

### 绘制图形

```cpp
void setup() {
    display.begin();
    display.clear(0x000000);
    
    // 绘制三个彩色矩形
    display.fillRect(10, 10, 50, 50, 0xFF0000);   // 红色
    display.fillRect(60, 10, 100, 50, 0x00FF00);  // 绿色
    display.fillRect(110, 10, 150, 50, 0x0000FF); // 蓝色
    
    display.flush();
}
```

### 像素级绘图

```cpp
void setup() {
    display.begin();
    display.clear(0x000000);
    
    // 绘制对角线
    uint16_t width = display.getWidth();
    uint16_t height = display.getHeight();
    
    for (int i = 0; i < min(width, height); i++) {
        display.drawPixel(i, i, 0xFFFFFF);  // 白色像素
    }
    
    display.flush();
}
```

### 颜色转换

```cpp
void setup() {
    display.begin();
    
    // 使用 RGB888 创建颜色
    uint8_t r = 255, g = 128, b = 64;
    uint16_t rgb565 = display.rgb888ToRgb565(r, g, b);
    
    // 转换为显示格式
    uint32_t displayColor = display.rgb565ToColor(rgb565);
    
    // 填充屏幕
    display.fillScreen(displayColor);
    display.flush();
}
```

## 底层 TDL 接口映射

Display 类封装了以下 TDL C 接口：

| Display 方法 | TDL 接口 |
|-------------|---------|
| `begin()` | `tdl_disp_find_dev()`, `tdl_disp_dev_open()`, `tdl_disp_create_frame_buff()` |
| `end()` | `tdl_disp_dev_close()`, `tdl_disp_free_frame_buff()` |
| `setBrightness()` | `tdl_disp_set_brightness()` |
| `flush()` | `tdl_disp_dev_flush()` |
| `clear()` / `fillScreen()` | `tdl_disp_draw_fill_full()` |
| `drawPixel()` | `tdl_disp_draw_point()` |
| `fillRect()` | `tdl_disp_draw_fill()` |
| `convertColor()` | `tdl_disp_convert_color_fmt()` |
| `rgb565ToColor()` | `tdl_disp_convert_rgb565_to_color()` |

## 支持的显示类型

根据板载配置自动支持：
- **ILI9488** - RGB 接口 LCD (TUYA_T5AI_BOARD_EX_MODULE_35565LCD)
- **ST7735S** - SPI 接口 LCD (TUYA_T5AI_BOARD_EX_MODULE_EYES)
- **ST7305** - E-Ink 显示 (TUYA_T5AI_BOARD_EX_MODULE_29E_INK)
- **SSD1306** - OLED 显示 (TUYA_T5AI_BOARD_EX_MODULE_096_OLED)

## 注意事项

1. **DISPLAY_NAME 宏**: 必须在编译时定义，指定要使用的显示设备名称
2. **内存分配**: 帧缓冲自动从 PSRAM 分配，大小根据显示分辨率和像素格式计算
3. **颜色格式**: 颜色值根据显示的像素格式自动转换（RGB565、RGB888、单色等）
4. **坐标系统**: 
   - 原点 (0,0) 在左上角
   - X 轴向右增长
   - Y 轴向下增长
5. **错误处理**: 所有操作返回 `OPERATE_RET`，`OPRT_OK` 表示成功

## 性能提示

- 批量绘制操作后调用一次 `flush()` 而不是每次绘制后都刷新
- 使用 `fillRect()` 比多次调用 `drawPixel()` 更高效
- 对于大量像素操作，考虑直接操作帧缓冲（高级用法）

## 错误代码

| 返回值 | 含义 |
|-------|------|
| `OPRT_OK` | 操作成功 |
| `OPRT_INVALID_PARM` | 无效参数（如坐标越界） |
| `OPRT_COM_ERROR` | 设备未初始化或通信错误 |
| `OPRT_MALLOC_FAILED` | 内存分配失败 |
| `OPRT_NOT_SUPPORTED` | 不支持的像素格式 |
