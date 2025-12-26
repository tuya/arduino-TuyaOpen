# Display Picture Example - 使用说明

## 概述

这个示例展示如何使用 Display C++ 类来显示图像，包括支持图像旋转功能。

## 功能特性

- **图像加载**: 从 RGB565 格式的图像数据创建帧缓冲
- **图像显示**: 将图像显示到屏幕
- **图像旋转**: 支持 90°、180°、270° 旋转
- **自动格式转换**: 自动将 RGB565 图像数据转换为显示器所需的像素格式

## 代码说明

### 简化的实现

原始代码使用了大量底层 TDL C 接口，现在已完全封装为 C++ 方法：

**原始 C 代码 (70+ 行)**:
```cpp
// 手动计算帧缓冲大小
bpp = tdl_disp_get_fmt_bpp(sg_display_info.fmt);
frame_len = width * height * bytes_per_pixel;

// 手动创建帧缓冲
fb = tdl_disp_create_frame_buff(DISP_FB_TP_PSRAM, frame_len);

// 逐像素转换和绘制
for (uint32_t j = 0; j < height; j++) {
    for (uint32_t i = 0; i < width; i++) {
        color = tdl_disp_convert_rgb565_to_color(color16, fb->fmt, 0x1000);
        tdl_disp_draw_point(fb, i, j, color, is_swap);
    }
}

// 手动处理旋转
if(sg_display_info.rotation != TUYA_DISPLAY_ROTATION_0) {
    fb_rotat = tdl_disp_create_frame_buff(...);
    tdl_disp_draw_rotate(rotation, sg_p_display_fb, fb_rotat, is_swap);
}
```

**新的 C++ 代码 (简洁)**:
```cpp
// 一行代码完成所有操作
display.drawImageRotated((const uint16_t*)imga_data, width, height, rotation);
```

### API 使用

#### 方法 1: 带旋转的图像显示（推荐）

```cpp
OPERATE_RET drawImageRotated(
    const uint16_t *imageData,    // RGB565 图像数据指针
    uint16_t imgWidth,             // 图像宽度
    uint16_t imgHeight,            // 图像高度
    TUYA_DISPLAY_ROTATION_E rotation  // 旋转角度
);
```

**示例**:
```cpp
// 自动处理旋转
TUYA_DISPLAY_ROTATION_E rotation = display.getRotation();
display.drawImageRotated((const uint16_t*)imga_data, imga_width, imga_height, rotation);
```

#### 方法 2: 手动管理帧缓冲（高级用法）

```cpp
// 1. 创建图像帧缓冲
TDL_DISP_FRAME_BUFF_T* createImageBuffer(
    const uint16_t *imageData,
    uint16_t imgWidth,
    uint16_t imgHeight
);

// 2. 刷新自定义帧缓冲
OPERATE_RET flushFrameBuffer(TDL_DISP_FRAME_BUFF_T *frameBuffer);
```

**示例**:
```cpp
// 创建图像缓冲
TDL_DISP_FRAME_BUFF_T *imageFb = display.createImageBuffer(
    (const uint16_t*)imga_data, 
    imga_width, 
    imga_height
);

if (imageFb != NULL) {
    // 显示图像
    display.flushFrameBuffer(imageFb);
    
    // 释放缓冲区
    tdl_disp_free_frame_buff(imageFb);
}
```

#### 方法 3: 在指定位置绘制图像

```cpp
OPERATE_RET drawImage(
    const uint16_t *imageData,
    uint16_t imgWidth,
    uint16_t imgHeight,
    uint16_t x = 0,      // X 坐标（默认 0）
    uint16_t y = 0       // Y 坐标（默认 0）
);
```

**示例**:
```cpp
// 在 (10, 20) 位置绘制小图标
display.drawImage(icon_data, 32, 32, 10, 20);
display.flush();
```

## 完整示例代码

```cpp
#include "Display.h"
#include "Log.h"

Display display;

extern const uint16_t imga_width;
extern const uint16_t imga_height;
extern const uint8_t imga_data[];

void setup() {
    Serial.begin(115200);
    Log.begin();
    
    // 初始化显示
    if (display.begin() != OPRT_OK) {
        PR_ERR("Display initialization failed");
        return;
    }
    
    PR_NOTICE("Display: %dx%d", display.getWidth(), display.getHeight());
    PR_NOTICE("Image: %dx%d", imga_width, imga_height);
    
    // 清屏
    display.clear(0x000000);
    display.flush();
}

void loop() {
    // 获取显示器旋转设置
    TUYA_DISPLAY_ROTATION_E rotation = display.getRotation();
    
    // 显示图像（自动处理旋转）
    if (display.drawImageRotated((const uint16_t*)imga_data, 
                                  imga_width, 
                                  imga_height, 
                                  rotation) == OPRT_OK) {
        PR_DEBUG("Image displayed");
    }
    
    delay(5000);
}
```

## 图像数据格式

图像数据必须是 RGB565 格式的 `uint16_t` 数组：

```cpp
// RGB565: RRRR RGGG GGGB BBBB (16 bits)
const uint16_t imageData[] = {
    0xF800,  // 红色 (R=31, G=0,  B=0)
    0x07E0,  // 绿色 (R=0,  G=63, B=0)
    0x001F,  // 蓝色 (R=0,  G=0,  B=31)
    // ... 更多像素
};

extern const uint16_t image_width = 320;
extern const uint16_t image_height = 240;
```

## 旋转角度

支持的旋转角度：

| 枚举值 | 旋转角度 | 说明 |
|--------|----------|------|
| `TUYA_DISPLAY_ROTATION_0` | 0° | 默认方向 |
| `TUYA_DISPLAY_ROTATION_90` | 90° | 顺时针旋转 90° |
| `TUYA_DISPLAY_ROTATION_180` | 180° | 顺时针旋转 180° |
| `TUYA_DISPLAY_ROTATION_270` | 270° | 顺时针旋转 270° |

## 性能对比

### 原始 C 实现
- **代码行数**: ~70 行
- **手动管理**: 帧缓冲创建、像素转换、旋转、释放
- **易错点**: 内存泄漏、错误的缓冲区大小计算

### 新 C++ 实现
- **代码行数**: ~5 行
- **自动管理**: 所有资源由 Display 类管理
- **类型安全**: 编译时检查参数类型
- **内存安全**: RAII 自动清理资源

## 内存管理

Display 类自动处理内存分配和释放：

```cpp
// ✅ 推荐：使用 drawImageRotated()
display.drawImageRotated(data, w, h, rotation);
// 内部自动创建和释放缓冲区

// ⚠️ 高级：手动管理时必须释放
TDL_DISP_FRAME_BUFF_T *fb = display.createImageBuffer(data, w, h);
display.flushFrameBuffer(fb);
tdl_disp_free_frame_buff(fb);  // 必须手动释放！
```

## 故障排除

### 1. 图像未显示

**检查**:
- 确认 `display.begin()` 成功
- 检查图像尺寸是否合理
- 验证图像数据指针非空

```cpp
if (display.begin() != OPRT_OK) {
    PR_ERR("Display init failed");
}
```

### 2. 图像显示颜色错误

**原因**: 像素格式不匹配
**解决**: Display 类自动转换 RGB565 到显示器格式，无需手动处理

### 3. 内存不足

**症状**: `createImageBuffer()` 返回 NULL
**解决**:
- 减小图像尺寸
- 检查 PSRAM 是否可用
- 及时释放不用的帧缓冲

```cpp
TDL_DISP_FRAME_BUFF_T *fb = display.createImageBuffer(data, w, h);
if (fb == NULL) {
    PR_ERR("Out of memory for image buffer");
}
```

## 性能提示

1. **批量操作**: 如需显示多张图像，创建一次缓冲后重复使用
2. **旋转开销**: 图像旋转需要额外的内存和 CPU 时间
3. **PSRAM 使用**: 大图像自动从 PSRAM 分配，速度稍慢但容量大

## 相关 API

- `display.getWidth()` - 获取显示器宽度
- `display.getHeight()` - 获取显示器高度
- `display.getRotation()` - 获取当前旋转角度
- `display.getPixelFormat()` - 获取像素格式
- `display.rgb565ToColor()` - RGB565 颜色转换
