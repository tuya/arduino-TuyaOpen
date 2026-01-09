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

#include "file.h"
#include "Log.h"

// SD Card mount point - can be customized
const char* SD_MOUNT_POINT = "/sdcard";

VFSFILE fs(SDCARD);

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
    if (!fs.exist("/")) {
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
    int ret = fs.mkdir("/test");
    Serial.println(ret == 0 ? "OK" : "FAILED");
    
    // Check if directory exists
    Serial.print("Checking if '/test' exists... ");
    ret = fs.exist("/test");
    Serial.println(ret == 1 ? "YES" : "NO");
    
    // Create a file
    Serial.print("Creating file '/test/hello.txt'... ");
    TUYA_FILE fd = fs.open("/test/hello.txt", "w");
    if (fd) {
        const char* msg = "Hello, SD Card!";
        fs.write(msg, strlen(msg), fd);
        fs.close(fd);
        Serial.println("OK");
    } else {
        Serial.println("FAILED");
    }
    
    // Get file size
    int size = fs.filesize("/test/hello.txt");
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
    TUYA_FILE fd = fs.open("/test/data.txt", "w");
    if (fd) {
        fs.write("Line 1: Hello World\n", 20, fd);
        fs.write("Line 2: SD Card Test\n", 21, fd);
        fs.write("Line 3: File Operations\n", 24, fd);
        fs.flush(fd);  // Ensure data is written to SD card
        fs.close(fd);
        Serial.println("OK");
    } else {
        Serial.println("FAILED");
        return;
    }
    
    // Read file content
    Serial.println("\nReading file content:");
    Serial.println("--------------------");
    fd = fs.open("/test/data.txt", "r");
    if (fd) {
        char buffer[64];
        while (!fs.feof(fd)) {
            memset(buffer, 0, sizeof(buffer));
            int bytes = fs.read(buffer, sizeof(buffer) - 1, fd);
            if (bytes > 0) {
                Serial.print(buffer);
            }
        }
        fs.close(fd);
        Serial.println("--------------------");
    }
    
    // Append to file
    Serial.print("\nAppending to file... ");
    fd = fs.open("/test/data.txt", "a");
    if (fd) {
        fs.write("Line 4: Appended line\n", 22, fd);
        fs.close(fd);
        Serial.println("OK");
    }
    
    // Read line by line
    Serial.println("\nReading line by line:");
    Serial.println("--------------------");
    fd = fs.open("/test/data.txt", "r");
    if (fd) {
        char line[64];
        while (!fs.feof(fd)) {
            memset(line, 0, sizeof(line));
            if (fs.readtillN(line, sizeof(line), fd) == 0) {
                Serial.print(line);
            }
        }
        fs.close(fd);
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
        TUYA_FILE fd = fs.open(path.c_str(), "w");
        if (fd) {
            char content[32];
            snprintf(content, sizeof(content), "Content of %s\n", files[i]);
            fs.write(content, strlen(content), fd);
            fs.close(fd);
        }
    }
    
    // List directory contents
    Serial.println("\nDirectory listing of '/test':");
    Serial.println("--------------------");
    TUYA_DIR dir = fs.openDir("/test");
    if (dir) {
        TUYA_FILEINFO info;
        while ((info = fs.readDir(dir)) != NULL) {
            const char *name;
            fs.getDirName(info, &name);
            
            // Skip "." and ".." entries
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
                continue;
            }
            
            String fullPath = String("/test/") + name;
            int size = fs.filesize(fullPath.c_str());
            
            Serial.print(name);
            Serial.print(" - ");
            Serial.print(size);
            Serial.println(" bytes");
        }
        fs.closeDir(dir);
        Serial.println("--------------------");
    } else {
        Serial.println("Failed to open directory");
    }
    
    // Clean up - remove files
    // Serial.println("\nCleaning up test files...");
    // for (int i = 0; i < 3; i++) {
    //     String path = String("/test/") + files[i];
    //     fs.remove(path.c_str());
    // }
    // fs.remove("/test/data.txt");
    // fs.remove("/test/hello.txt");
    // fs.remove("/test");
    // Serial.println("Cleanup completed");
}
