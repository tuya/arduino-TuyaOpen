# LVGLdemo - LVGL v9.1.0 Hello World

## 概述
本示例通过 Tuya 的 LVGL vendor 层初始化 LVGL v9.1.0，创建居中的 "Hello World!" 标签，并启动 LVGL 任务。

## 硬件要求
- TUYA_T5AI 开发板（已接入并支持的显示屏）

## 使用方法
1. 上传示例程序。
2. 屏幕会显示白底，并在中间显示 "Hello World!"。

## 说明
- LVGL 任务启动方式：

```cpp
lv_vendor_start(5, 1024*8);
```
