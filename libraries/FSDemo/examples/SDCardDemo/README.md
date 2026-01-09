# SD Card Demo Example


## Overview
This example demonstrates comprehensive SD card file system operations using the Tuya FS library. It covers basic file operations, directory management, and advanced file handling techniques.

## Features
- **File Operations**: Create, read, write, and delete files
- **Directory Management**: Create directories, list contents, navigate structure
- **File Information**: Get file size, check existence, and file attributes
- **Data Writing**: Write strings and binary data to files
- **Data Reading**: Read entire files or specific portions
- **Error Handling**: Robust error checking and reporting

## Hardware Requirements
- Tuya development board with SD card slot
- SD card (formatted as FAT32)
- USB cable for power and serial communication

## SD Card Preparation
1. Format SD card as FAT32
2. Insert SD card into the board's SD card slot
3. Ensure card is properly seated before powering on

## Usage Instructions

### 1. Upload the Sketch
- Open `SDCardDemo.ino` in Arduino IDE
- Select your Tuya board
- Upload the sketch

### 2. Open Serial Monitor
- Set baud rate to 115200
- View detailed operation logs

### 3. Observe Demo Execution
The demo automatically executes the following tests:
- Test 1: Basic file creation and writing
- Test 2: File reading operations
- Test 3: Directory operations and listing
- Test 4: File information retrieval

## Demonstration Features

### 1. Basic File Operations
```cpp
// Create directory
fs.mkdir("/test_dir");

// Create and write file
TUYA_FILE file = fs.open("/test_dir/test.txt", "w");
fs.write("Hello, SD Card!", 15, file);
fs.close(file);
```

### 2. File Reading
```cpp
// Read entire file
TUYA_FILE file = fs.open("/test_dir/test.txt", "r");
char buffer[100];
int bytes_read = fs.read(buffer, sizeof(buffer), file);
```

### 3. Directory Operations
```cpp
// List directory contents
TUYA_DIR dir = fs.openDir("/test_dir");
TUYA_FILEINFO info;
while ((info = fs.readDir(dir)) != NULL) {
    const char *name;
    fs.getDirName(info, &name);
    // Process file/directory
}
fs.closeDir(dir);
```

### 4. File Information
```cpp
// Check file existence
bool exists = fs.exist("/test_dir/test.txt");

// Get file size
int size = fs.filesize("/test_dir/test.txt");
```

## Serial Monitor Output Example
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

## Key Functions Demonstrated

### File Management
- `mkdir()` - Create directory
- `remove()` - Delete file or empty directory
- `rename()` - Rename file or directory
- `exist()` - Check if file/directory exists

### File Operations
- `open()` - Open file for reading/writing
- `close()` - Close file handle
- `read()` - Read data from file
- `write()` - Write data to file
- `flush()` - Flush write buffer
- `feof()` - Check end of file
- `filesize()` - Get file size
- `lseek()` - Seek to file position

### Directory Operations
- `openDir()` - Open directory for reading
- `closeDir()` - Close directory handle
- `readDir()` - Read next directory entry
- `getDirName()` - Get entry name
- `isDirectory()` - Check if entry is directory

## File Modes
- `"r"` - Read only
- `"w"` - Write (creates new file, truncates existing)
- `"a"` - Append (write to end of file)
- `"r+"` - Read and write
- `"w+"` - Read and write (truncates existing)

## Troubleshooting

**SD card not detected:**
- Check SD card is inserted properly
- Verify SD card is formatted as FAT32
- Try different SD card
- Check Serial Monitor message: "SD card not mounted!"

**File operations fail:**
- Check if path starts with `/`
- Verify parent directory exists before creating files
- Ensure SD card has sufficient space
- Check for write-protection on SD card

**Directory listing shows no files:**
- Verify files were created successfully
- Check correct directory path
- Ensure files weren't deleted by previous operations

**Cannot write to files:**
- SD card may be full
- SD card may be write-protected
- File may be opened in read-only mode
- Check return values for error codes

## Advanced Usage

### Binary File Writing
```cpp
uint8_t data[1024];
// Fill data array
TUYA_FILE file = fs.open("/data.bin", "w");
fs.write((const char*)data, sizeof(data), file);
fs.close(file);
```

### Reading Line by Line
```cpp
char line[256];
TUYA_FILE file = fs.open("/text.txt", "r");
while (!fs.feof(file)) {
    int len = fs.readtillN(line, sizeof(line)-1, file);
    if (len > 0) {
        line[len] = '\0';
        // Process line
    }
}
fs.close(file);
```

### File Position Control
```cpp
TUYA_FILE file = fs.open("/data.bin", "r");
fs.lseek(file, 100, SEEK_SET);  // Seek to position 100
int pos = fs.position(file);     // Get current position
```

## Code Structure
The demo is organized into test functions:
- `test_basic_operations()` - File creation and writing
- `test_file_operations()` - Reading and data retrieval
- `test_directory_operations()` - Directory listing and navigation

Each function demonstrates specific aspects of SD card file operations with detailed logging.

## Dependencies
- **FS Library**: Tuya file system abstraction
- **Log Library**: Serial logging utilities

## Notes
- All file paths must start with `/`
- Maximum path length is typically 255 characters
- File names are case-sensitive on some systems
- Always close files after operations to ensure data is written
- Use `flush()` to force write buffer to SD card

## Related Examples
- `LittleFSDemo` - Internal flash file system operations
- `audio_output_sdcard` - Audio recording to SD card
- `audio_speaker` - Play audio files from SD card

## Best Practices
1. **Always check return values** for error handling
2. **Close files promptly** to free resources
3. **Use flush()** for critical writes
4. **Check file existence** before operations
5. **Handle errors gracefully** with appropriate messages

## SD Card Recommendations
- Use reliable brand SD cards
- Format as FAT32 for best compatibility
- Test card in computer before use
