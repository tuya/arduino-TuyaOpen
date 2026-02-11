/**
 * @file SDCardDemo.ino
 * @brief SD Card basic operations demo
 * 
 * This example demonstrates basic SD card file operations including:
 * - File creation and writing
 * - File reading
 * - Directory operations
 * - File information retrieval
 * 
 * @note ===================== Chat bot only support TUYA_T5AI platform =====================
 */

#include "File.h"
#include "Log.h"

// SD Card mount point - can be customized
const char* SD_MOUNT_POINT = "/sdcard";

VFSFILE SD(SDCARD);

void test_basic_operations();
void test_file_operations();
void test_directory_operations();

void setup() {
    // Initialize logging
    Serial.begin(115200);
    Log.begin();
    delay(1000);
    
    Serial.println("\n=== SD Card Demo ===\n");
    Serial.print("SD Card mount point: ");
    Serial.println(SD_MOUNT_POINT);
    Serial.println();
    
    // Check if SD card is mounted by checking root path
    if (!SD.exist("/")) {
        Serial.println("ERROR: SD card not mounted!");
        Serial.println("Please check if SD card is inserted properly.");
        return;
    }
    
    Serial.println("SD card mounted successfully!");
    Serial.println();
    
    // Test 1: Create directory and file
    test_basic_operations();
    
    Serial.println();
    
    // Test 2: Read and write operations
    test_file_operations();
    
    Serial.println();
    
    // Test 3: Directory listing
    test_directory_operations();
    
    Serial.println("\n=== All tests completed ===");
}

void loop() {
    delay(1000);
}

/**
 * @brief Test basic file system operations
 */
void test_basic_operations() {
    Serial.println("--- Test 1: Basic Operations ---");
    
    // Create directory
    Serial.print("Creating directory '/test'... ");
    int ret = SD.mkdir("/test");
    Serial.println(ret == 0 ? "OK" : "FAILED");
    
    // Check if directory exists
    Serial.print("Checking if '/test' exists... ");
    ret = SD.exist("/test");
    Serial.println(ret == 1 ? "YES" : "NO");
    
    // Create a file
    Serial.print("Creating file '/test/hello.txt'... ");
    TUYA_FILE fd = SD.open("/test/hello.txt", "w");
    if (fd) {
        const char* msg = "Hello, SD Card!";
        SD.write(msg, strlen(msg), fd);
        SD.close(fd);
        Serial.println("OK");
    } else {
        Serial.println("FAILED");
    }
    
    // Get file size
    int size = SD.filesize("/test/hello.txt");
    Serial.print("File size: ");
    Serial.print(size);
    Serial.println(" bytes");
}

/**
 * @brief Test file read and write operations
 */
void test_file_operations() {
    Serial.println("--- Test 2: File Operations ---");
    
    // Write multiple lines to a file
    Serial.print("Writing to '/test/data.txt'... ");
    TUYA_FILE fd = SD.open("/test/data.txt", "w");
    if (fd) {
        SD.write("Line 1: Hello World\n", 20, fd);
        SD.write("Line 2: SD Card Test\n", 21, fd);
        SD.write("Line 3: File Operations\n", 24, fd);
        SD.flush(fd);  // Ensure data is written to SD card
        SD.close(fd);
        Serial.println("OK");
    } else {
        Serial.println("FAILED");
        return;
    }
    
    // Read file content
    Serial.println("\nReading file content:");
    Serial.println("--------------------");
    fd = SD.open("/test/data.txt", "r");
    if (fd) {
        char buffer[64];
        while (!SD.feof(fd)) {
            memset(buffer, 0, sizeof(buffer));
            int bytes = SD.read(buffer, sizeof(buffer) - 1, fd);
            if (bytes > 0) {
                Serial.print(buffer);
            }
        }
        SD.close(fd);
        Serial.println("--------------------");
    }
    
    // Append to file
    Serial.print("\nAppending to file... ");
    fd = SD.open("/test/data.txt", "a");
    if (fd) {
        SD.write("Line 4: Appended line\n", 22, fd);
        SD.close(fd);
        Serial.println("OK");
    }
    
    // Read line by line
    Serial.println("\nReading line by line:");
    Serial.println("--------------------");
    fd = SD.open("/test/data.txt", "r");
    if (fd) {
        char line[64];
        while (!SD.feof(fd)) {
            memset(line, 0, sizeof(line));
            if (SD.readtillN(line, sizeof(line), fd) == 0) {
                Serial.print(line);
            }
        }
        SD.close(fd);
        Serial.println("--------------------");
    }
}

/**
 * @brief Test directory operations
 */
void test_directory_operations() {
    Serial.println("--- Test 3: Directory Operations ---");
    
    // Create multiple files
    const char* files[] = {"file1.txt", "file2.txt", "file3.txt"};
    for (int i = 0; i < 3; i++) {
        String path = String("/test/") + files[i];
        TUYA_FILE fd = SD.open(path.c_str(), "w");
        if (fd) {
            char content[32];
            snprintf(content, sizeof(content), "Content of %s\n", files[i]);
            SD.write(content, strlen(content), fd);
            SD.close(fd);
        }
    }
    
    // List directory contents
    Serial.println("\nDirectory listing of '/test':");
    Serial.println("--------------------");
    TUYA_DIR dir = SD.openDir("/test");
    if (dir) {
        TUYA_FILEINFO info;
        while ((info = SD.readDir(dir)) != NULL) {
            const char *name;
            SD.getDirName(info, &name);
            
            // Skip "." and ".." entries
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
                continue;
            }
            
            String fullPath = String("/test/") + name;
            int size = SD.filesize(fullPath.c_str());
            
            Serial.print(name);
            Serial.print(" - ");
            Serial.print(size);
            Serial.println(" bytes");
        }
        SD.closeDir(dir);
        Serial.println("--------------------");
    } else {
        Serial.println("Failed to open directory");
    }
    
    // Clean up - remove files
    // Serial.println("\nCleaning up test files...");
    // for (int i = 0; i < 3; i++) {
    //     String path = String("/test/") + files[i];
    //     SD.remove(path.c_str());
    // }
    // SD.remove("/test/data.txt");
    // SD.remove("/test/hello.txt");
    // SD.remove("/test");
    // Serial.println("Cleanup completed");
}
