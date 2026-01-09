# Ticker 参数传递示例

## 概述

本示例演示了如何向 Ticker 回调函数传递参数。展示了如何对多个定时器使用相同的回调函数和不同的参数。

## 功能特性

- 向定时器回调传递参数
- 多个定时器使用相同回调函数
- 不同的计时间隔
- 回调中的参数区分
- 非阻塞操作

## 硬件要求

- 涂鸦支持的开发板
- 用于输出的串口监视器

## 工作原理

1. **定时器 1**: 每 1 秒调用回调，参数值为 1
2. **定时器 2**: 每 5 秒调用回调，参数值为 5
3. **回调**: 打印参数值以区分哪个定时器触发

## 带参数的 Ticker

### 语法
```cpp
ticker.attach(interval, callback, parameter);
```

- `interval`: 回调之间的时间间隔（秒）
- `callback`: 要调用的函数
- `parameter`: 参数数据的指针

## 代码说明

### 设置
```cpp
int arg_1s = 1;
Ticker ticker_1s;

int arg_5s = 5;
Ticker ticker_5s;

ticker_1s.attach(1, ticker_callback, &arg_1s);
ticker_5s.attach(5, ticker_callback, &arg_5s);
```

### 回调函数
```cpp
void ticker_callback(int *arg)
{
  Serial.print("arg: ");
  Serial.println(*arg);
}
```

## 使用方法

1. 将固件烧录到设备
2. 打开串口监视器，波特率 115200
3. 观察输出：
   - "arg: 1" 每 1 秒出现一次
   - "arg: 5" 每 5 秒出现一次

## 示例输出

```
arg: 1
arg: 1
arg: 1
arg: 1
arg: 5
arg: 1
arg: 1
...
```

## 参数类型

可以传递不同类型的参数：

### 整数
```cpp
int value = 42;
ticker.attach(1.0, callback, &value);

void callback(int *arg) {
  Serial.println(*arg);
}
```

### 浮点数
```cpp
float value = 3.14;
ticker.attach(1.0, callback, &value);

void callback(float *arg) {
  Serial.println(*arg);
}
```

### 结构体
```cpp
struct Data {
  int id;
  float value;
};

Data data = {1, 23.5};
ticker.attach(1.0, callback, &data);

void callback(Data *arg) {
  Serial.println(arg->id);
  Serial.println(arg->value);
}
```

### 字符串/数组
```cpp
char message[] = "Hello";
ticker.attach(1.0, callback, message);

void callback(char *arg) {
  Serial.println(arg);
}
```

## 高级示例

### 多个传感器
```cpp
struct SensorConfig {
  int pin;
  const char* name;
  float threshold;
};

SensorConfig temp = {A0, "Temperature", 25.0};
SensorConfig hum = {A1, "Humidity", 60.0};

Ticker tempTicker;
Ticker humTicker;

void setup() {
  tempTicker.attach(2, readSensor, &temp);
  humTicker.attach(5, readSensor, &hum);
}

void readSensor(SensorConfig *config) {
  int value = analogRead(config->pin);
  Serial.print(config->name);
  Serial.print(": ");
  Serial.println(value);
}
```

## 用例

- 多个传感器具有不同的读取间隔
- 不同的 LED 闪烁模式使用相同回调
- 带定时器触发状态的状态机
- 多通道数据记录
- 带任务特定参数的计划任务

## 重要注意事项

### 参数生命周期
- 参数必须在定时器操作期间保持有效
- 不要使用超出作用域的局部变量
- 使用全局或静态变量

✅ **正确**:
```cpp
int globalValue = 10;
ticker.attach(1, callback, &globalValue);
```

❌ **错误**:
```cpp
void setup() {
  int localValue = 10;  // setup() 后将被销毁
  ticker.attach(1, callback, &localValue);  // 错误！
}
```

### 线程安全
- 小心处理共享数据
- 对回调中访问的变量使用 volatile
- 考虑为关键部分禁用中断

### 内存管理
- 首选静态分配
- 避免在回调中动态分配
- 确保定时器活动时参数内存未被释放

## 回调最佳实践

✅ **推荐**:
- 保持回调简短快速
- 使用参数进行配置
- 为主循环处理设置标志
- 对共享变量使用 volatile

❌ **避免**:
- 执行冗长操作
- 过度使用 Serial.print()
- 调用 delay()
- 动态分配内存

## 故障排除

- **错误的值**: 检查参数指针
- **崩溃**: 验证参数生命周期
- **无输出**: 确保串口已初始化
- **时序不正确**: 检查回调是否太慢

## 依赖库

- Ticker 库（已包含）

## 相关示例

- Blinker: 不带参数的基本 Ticker 用法
- Multiple timers: 管理多个独立定时器

## 注意事项

- 参数通过指针（引用）传递
- 回调函数必须匹配参数类型
- 多个定时器可以使用不同参数共享相同回调
- 此模式支持代码重用并减少重复
