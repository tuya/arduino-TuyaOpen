# SD卡演示示例


## 概述
本示例演示使用涂鸦FS库进行全面的SD卡文件系统操作。涵盖基本文件操作、目录管理和高级文件处理技术。

## 功能特性
- **文件操作**: 创建、读取、写入和删除文件
- **目录管理**: 创建目录、列出内容、导航结构
- **文件信息**: 获取文件大小、检查存在性和文件属性
- **数据写入**: 将字符串和二进制数据写入文件
- **数据读取**: 读取整个文件或特定部分
- **错误处理**: 健壮的错误检查和报告

## 硬件要求
- 带SD卡槽的涂鸦开发板
- SD卡(格式化为FAT32)
- USB线用于供电和串口通信

## SD卡准备
1. 将SD卡格式化为FAT32
2. 将SD卡插入开发板的SD卡槽
3. 确保卡片在上电前正确就位

## 使用说明

### 1. 上传程序
- 在Arduino IDE中打开`SDCardDemo.ino`
- 选择你的涂鸦开发板
- 上传程序

### 2. 打开串口监视器
- 设置波特率为115200
- 查看详细操作日志

### 3. 观察演示执行
演示自动执行以下测试:
- 测试1: 基本文件创建和写入
- 测试2: 文件读取操作
- 测试3: 目录操作和列表
- 测试4: 文件信息检索

## 演示功能

### 1. 基本文件操作
```cpp
// 创建目录
SD.mkdir("/test_dir");

// 创建并写入文件
TUYA_FILE file = SD.open("/test_dir/test.txt", "w");
SD.write("Hello, SD Card!", 15, file);
SD.close(file);
```

### 2. 文件读取
```cpp
// 读取整个文件
TUYA_FILE file = SD.open("/test_dir/test.txt", "r");
char buffer[100];
int bytes_read = SD.read(buffer, sizeof(buffer), file);
```

### 3. 目录操作
```cpp
// 列出目录内容
TUYA_DIR dir = SD.openDir("/test_dir");
TUYA_FILEINFO info;
while ((info = SD.readDir(dir)) != NULL) {
    const char *name;
    SD.getDirName(info, &name);
    // 处理文件/目录
}
SD.closeDir(dir);
```

### 4. 文件信息
```cpp
// 检查文件存在性
bool exists = SD.exist("/test_dir/test.txt");

// 获取文件大小
int size = SD.filesize("/test_dir/test.txt");
```

## 串口监视器输出示例
```
=== SD Card Demo ===

SD Card mount point: /sdcard

SD card mounted successfully!

=== Test 1: Basic File Operations ===
Creating directory: /test_dir
Directory created successfully

Creating file: /test_dir/hello.txt
Writing: Hello, SD Card!
File created and written successfully

=== Test 2: File Reading ===
Reading file: /test_dir/hello.txt
File content: Hello, SD Card!
File size: 15 bytes

=== Test 3: Directory Operations ===
Listing /test_dir contents:
  [FILE] hello.txt (15 bytes)
  [FILE] data.bin (1024 bytes)

=== Test 4: File Information ===
Checking file: /test_dir/hello.txt
  Exists: Yes
  Size: 15 bytes
  Type: Regular file

=== All tests completed ===
```

## 演示的关键函数

### 文件管理
- `mkdir()` - 创建目录
- `remove()` - 删除文件或空目录
- `rename()` - 重命名文件或目录
- `exist()` - 检查文件/目录是否存在

### 文件操作
- `open()` - 打开文件进行读/写
- `close()` - 关闭文件句柄
- `read()` - 从文件读取数据
- `write()` - 向文件写入数据
- `flush()` - 刷新写缓冲区
- `feof()` - 检查文件结尾
- `filesize()` - 获取文件大小
- `lseek()` - 定位到文件位置

### 目录操作
- `openDir()` - 打开目录进行读取
- `closeDir()` - 关闭目录句柄
- `readDir()` - 读取下一个目录项
- `getDirName()` - 获取项名称
- `isDirectory()` - 检查项是否为目录

## 文件模式
- `"r"` - 只读
- `"w"` - 写入(创建新文件,截断现有文件)
- `"a"` - 追加(写到文件末尾)
- `"r+"` - 读写
- `"w+"` - 读写(截断现有文件)

## 故障排除

**SD卡未检测到:**
- 检查SD卡是否正确插入
- 验证SD卡格式化为FAT32
- 尝试不同的SD卡
- 检查串口监视器消息: "SD card not mounted!"

**文件操作失败:**
- 检查路径是否以`/`开头
- 在创建文件前验证父目录存在
- 确保SD卡有足够空间
- 检查SD卡是否写保护

**目录列表不显示文件:**
- 验证文件已成功创建
- 检查正确的目录路径
- 确保文件未被之前的操作删除

**无法写入文件:**
- SD卡可能已满
- SD卡可能被写保护
- 文件可能以只读模式打开
- 检查返回值的错误代码

## 高级用法

### 二进制文件写入
```cpp
uint8_t data[1024];
// 填充数据数组
TUYA_FILE file = SD.open("/data.bin", "w");
SD.write((const char*)data, sizeof(data), file);
SD.close(file);
```

### 逐行读取
```cpp
char line[256];
TUYA_FILE file = SD.open("/text.txt", "r");
while (!SD.feof(file)) {
    int len = SD.readtillN(line, sizeof(line)-1, file);
    if (len > 0) {
        line[len] = '\0';
        // 处理行
    }
}
SD.close(file);
```

### 文件位置控制
```cpp
TUYA_FILE file = SD.open("/data.bin", "r");
SD.lseek(file, 100, SEEK_SET);  // 定位到位置100
int pos = SD.position(file);     // 获取当前位置
```

## 代码结构
演示组织为测试函数:
- `test_basic_operations()` - 文件创建和写入
- `test_file_operations()` - 读取和数据检索
- `test_directory_operations()` - 目录列表和导航

每个函数演示SD卡文件操作的特定方面,并提供详细日志。

## 依赖库
- **FS库**: 涂鸦文件系统抽象
- **Log库**: 串口日志工具

## 注意事项
- 所有文件路径必须以`/`开头
- 最大路径长度通常为255个字符
- 文件名在某些系统上区分大小写
- 操作后始终关闭文件以确保数据被写入
- 使用`flush()`强制将写缓冲区写入SD卡

## 相关示例
- `LittleFSDemo` - 内部闪存文件系统操作
- `audio_output_sdcard` - 录音到SD卡
- `audio_speaker` - 从SD卡播放音频文件

## 最佳实践
1. **始终检查返回值** 进行错误处理
2. **及时关闭文件** 以释放资源
3. **使用flush()** 进行关键写入
4. **检查文件存在性** 再进行操作
5. **优雅处理错误** 并提供适当的消息

## SD卡建议
- 使用可靠品牌的SD卡
- 格式化为FAT32以获得最佳兼容性
- 使用前在电脑上测试卡
