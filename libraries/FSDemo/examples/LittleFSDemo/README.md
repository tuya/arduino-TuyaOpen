# LittleFS Demo

## Description

This example demonstrates the comprehensive usage of the LittleFS (Little File System) library on Tuya boards. It showcases file operations, directory management, and file system manipulation capabilities through three test cases.

## Hardware Requirements

- Any Tuya-supported development board (TUYA_T5AI)
- USB cable for serial connection

## Usage Instructions

1. Open the example in Arduino IDE
2. Select your Tuya board from Tools > Board menu
3. Upload the sketch to your board
4. Open Serial Monitor at 115200 baud rate
5. Observe the test results showing file operations, directory operations, and file copying

## Key Features

### Test 1: Basic File Operations
- **File Creation & Writing**: Creates a text file and writes content to it
- **File Reading**: Reads data from file in various ways (by size, character by character, until newline)
- **File Size**: Retrieves file size information
- **File Seeking**: Demonstrates seeking to specific positions in a file
- **Position Tracking**: Shows current read/write position in the file

### Test 2: Directory Operations
- **Directory Creation**: Creates nested directories (`mkdir`)
- **Directory Existence Check**: Verifies if a directory exists
- **Directory Listing**: Opens and reads directory contents
- **Directory Renaming**: Renames directories
- **Directory Removal**: Removes directories and subdirectories

### Test 3: File Copying
- **Multi-file Handling**: Opens multiple files simultaneously
- **File Reading & Writing**: Reads from one file and writes to another
- **EOF Detection**: Detects end-of-file while reading
- **Buffered I/O**: Uses buffer for efficient file copying

## Code Output

The serial monitor will display:
- File sizes and positions
- Directory operation results (success/failure)
- File content read from various operations
- Continuous status messages showing the sketch is running
