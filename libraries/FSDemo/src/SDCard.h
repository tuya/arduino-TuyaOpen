/**
 * @file SDCard.h
 * @brief SD Card file system implementation for Tuya Arduino platform
 * @version 1.0
 * @date 2026-01-07
 *
 * @copyright Copyright (c) 2026 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef _SDCARD_H_
#define _SDCARD_H_

#include <Arduino.h>
#include "fs_api.h"

extern "C" {
#include "tkl_fs.h"
#include "tal_log.h"
}

/**
 * @brief SD Card file system implementation class
 * 
 * This class provides SD card file system operations by wrapping
 * the TKL (Tuya Kernel Layer) file system APIs. It supports FatFS
 * on SD cards and handles automatic path resolution with mount points.
 */
class FS_SDCARD : public FS_API
{
public:
    /**
     * @brief Construct a new FS_SDCARD object with default mount point
     * 
     * Automatically mounts the SD card to "/sdcard"
     */
    FS_SDCARD();
    
    /**
     * @brief Construct a new FS_SDCARD object with custom mount point
     * 
     * @param mount_point Custom mount point path (e.g., "/sd", "/mnt/sdcard")
     */
    FS_SDCARD(const char* mount_point);
    
    /**
     * @brief Destroy the FS_SDCARD object
     * 
     * Automatically unmounts the SD card if it was mounted
     */
    ~FS_SDCARD();

    // Directory operations
    virtual int mkdir(const char *path);
    virtual int remove(const char *path);
    virtual int exist(const char *path);
    virtual int rename(const char *pathFrom, const char *pathTo);
    
    // Directory traversal
    virtual TUYA_DIR openDir(const char *path);
    virtual int closeDir(TUYA_DIR dir);
    virtual TUYA_FILEINFO readDir(TUYA_DIR dir);
    virtual int getDirName(TUYA_FILEINFO info, const char** name);
    virtual int isDirectory(const char *path);

    // File operations
    virtual TUYA_FILE open(const char *path);
    virtual TUYA_FILE open(const char *path, const char* mode);
    virtual int close(TUYA_FILE fd);
    virtual char read(TUYA_FILE fd);
    virtual int read(const char *buf, int size, TUYA_FILE fd);
    virtual int readtillN(char *buf, int size, TUYA_FILE fd);
    virtual int write(const char *buf, int size, TUYA_FILE fd);
    virtual void flush(TUYA_FILE fd);
    virtual int feof(TUYA_FILE fd);
    virtual int lseek(TUYA_FILE fd, int offs, int whence);
    virtual int position(TUYA_FILE fd);
    virtual int filesize(const char *filepath);
    
    /**
     * @brief Mount the SD card file system
     * 
     * @param mount_point Mount point path (optional, uses default if NULL)
     * @return true if mount successful
     * @return false if mount failed
     */
    bool mount(const char* mount_point = NULL);
    
    /**
     * @brief Unmount the SD card file system
     * 
     * @return true if unmount successful
     * @return false if unmount failed
     */
    bool unmount();
    
    /**
     * @brief Check if SD card is mounted
     * 
     * @return true if mounted
     * @return false if not mounted
     */
    bool isMounted() { return ismounted; }
    
    /**
     * @brief Get the current mount point
     * 
     * @return const char* Mount point path
     */
    const char* getMountPoint() { return mount_point; }
    
protected:
    bool ismounted;           ///< Mount status flag
    char mount_point[32];     ///< Mount point path
    
    /**
     * @brief Convert relative path to absolute path with mount point
     * 
     * @param path Input path (relative or absolute)
     * @return String Full path with mount point prefix
     */
    String getFullPath(const char* path);
};

#endif // _SDCARD_H_
