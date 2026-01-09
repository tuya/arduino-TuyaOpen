/**
 * @file SDCard.cpp
 * @brief SD Card file system implementation
 * @version 1.0
 * @date 2026-01-07
 *
 * @copyright Copyright (c) 2026 Tuya Inc. All Rights Reserved.
 *
 */

#include "SDCard.h"

FS_SDCARD::FS_SDCARD() : ismounted(false) {
    strcpy(mount_point, "/sdcard");
    mount();
}

FS_SDCARD::FS_SDCARD(const char* mp) : ismounted(false) {
    strncpy(mount_point, mp, sizeof(mount_point) - 1);
    mount_point[sizeof(mount_point) - 1] = '\0';
    mount();
}

FS_SDCARD::~FS_SDCARD() {
    if (ismounted) {
        unmount();
    }
}

bool FS_SDCARD::mount(const char* mp) {
    if (ismounted) {
        return true;
    }
    
    if (mp) {
        strncpy(mount_point, mp, sizeof(mount_point) - 1);
        mount_point[sizeof(mount_point) - 1] = '\0';
    }
    
    // Use TKL layer to mount SD card with FatFS
    int ret = tkl_fs_mount(mount_point, DEV_SDCARD);
    if (ret == 0) {
        ismounted = true;
        PR_DEBUG("SD card mounted at %s", mount_point);
    } else {
        PR_ERR("Failed to mount SD card at %s, ret=%d", mount_point, ret);
    }
    
    return ismounted;
}

bool FS_SDCARD::unmount() {
    if (!ismounted) {
        return true;
    }
    
    int ret = tkl_fs_unmount(mount_point);
    if (ret == 0) {
        ismounted = false;
        PR_DEBUG("SD card unmounted");
        return true;
    }
    
    PR_ERR("Failed to unmount SD card, ret=%d", ret);
    return false;
}

String FS_SDCARD::getFullPath(const char* path) {
    // If path already contains mount point, return as is
    if (strncmp(path, mount_point, strlen(mount_point)) == 0) {
        return String(path);
    }
    
    // Otherwise, add mount point prefix
    String fullPath = mount_point;
    if (path[0] != '/') {
        fullPath += "/";
    }
    fullPath += path;
    
    return fullPath;
}

int FS_SDCARD::mkdir(const char *path) {
    if (!ismounted) {
        return -1;
    }
    
    String fullPath = getFullPath(path);
    return tkl_fs_mkdir(fullPath.c_str());
}

int FS_SDCARD::remove(const char *path) {
    if (!ismounted) {
        return -1;
    }
    
    String fullPath = getFullPath(path);
    return tkl_fs_remove(fullPath.c_str());
}

int FS_SDCARD::exist(const char *path) {
    if (!ismounted) {
        return 0;
    }
    
    // Special handling for root path - check if mounted
    if (strcmp(path, "/") == 0) {
        return ismounted ? 1 : 0;
    }
    
    String fullPath = getFullPath(path);
    BOOL_T is_exist = false;
    int ret = tkl_fs_is_exist(fullPath.c_str(), &is_exist);
    
    if (ret != 0) {
        return 0;
    }
    
    return is_exist ? 1 : 0;
}

int FS_SDCARD::rename(const char *pathFrom, const char *pathTo) {
    if (!ismounted) {
        return -1;
    }
    
    String fullPathFrom = getFullPath(pathFrom);
    String fullPathTo = getFullPath(pathTo);
    
    return tkl_fs_rename(fullPathFrom.c_str(), fullPathTo.c_str());
}

TUYA_DIR FS_SDCARD::openDir(const char *path) {
    if (!ismounted) {
        return NULL;
    }
    
    String fullPath = getFullPath(path);
    TUYA_DIR dir;
    
    int ret = tkl_dir_open(fullPath.c_str(), &dir);
    if (ret != 0) {
        return NULL;
    }
    
    return dir;
}

int FS_SDCARD::closeDir(TUYA_DIR dir) {
    if (!ismounted || !dir) {
        return -1;
    }
    
    return tkl_dir_close(dir);
}

TUYA_FILEINFO FS_SDCARD::readDir(TUYA_DIR dir) {
    if (!ismounted || !dir) {
        return NULL;
    }
    
    TUYA_FILEINFO info = NULL;
    int ret = tkl_dir_read(dir, &info);
    
    if (ret != 0) {
        return NULL;
    }
    
    return info;
}

int FS_SDCARD::getDirName(TUYA_FILEINFO info, const char** name) {
    if (!ismounted || !info) {
        return -1;
    }
    
    return tkl_dir_name(info, name);
}

int FS_SDCARD::isDirectory(const char *path) {
    if (!ismounted) {
        return -1;
    }
    
    String fullPath = getFullPath(path);
    TUYA_DIR dir;
    
    int ret = tkl_dir_open(fullPath.c_str(), &dir);
    if (ret != 0) {
        return 0;
    }
    
    TUYA_FILEINFO info = NULL;
    ret = tkl_dir_read(dir, &info);
    if (ret != 0 || !info) {
        tkl_dir_close(dir);
        return 0;
    }
    
    BOOL_T is_dir = false;
    tkl_dir_is_directory(info, &is_dir);
    tkl_dir_close(dir);
    
    return is_dir ? 1 : 0;
}

TUYA_FILE FS_SDCARD::open(const char *path) {
    return open(path, "a+");
}

TUYA_FILE FS_SDCARD::open(const char *path, const char* mode) {
    if (!ismounted) {
        return NULL;
    }
    
    String fullPath = getFullPath(path);
    return tkl_fopen(fullPath.c_str(), mode);
}

int FS_SDCARD::close(TUYA_FILE fd) {
    if (!ismounted || !fd) {
        return -1;
    }
    
    return tkl_fclose(fd);
}

char FS_SDCARD::read(TUYA_FILE fd) {
    if (!ismounted || !fd) {
        return '\0';
    }
    
    return (char)tkl_fgetc(fd);
}

int FS_SDCARD::read(const char *buf, int size, TUYA_FILE fd) {
    if (!ismounted || !fd || !buf || !size) {
        return -1;
    }
    
    return tkl_fread((void*)buf, size, fd);
}

int FS_SDCARD::readtillN(char *buf, int size, TUYA_FILE fd) {
    if (!ismounted || !fd || !buf || !size) {
        return -1;
    }
    
    char* result = tkl_fgets(buf, size, fd);
    return (result != NULL) ? 0 : -1;
}

int FS_SDCARD::write(const char *buf, int size, TUYA_FILE fd) {
    if (!ismounted || !fd || !buf || !size) {
        return -1;
    }
    
    return tkl_fwrite((void*)buf, size, fd);
}

void FS_SDCARD::flush(TUYA_FILE fd) {
    if (!ismounted || !fd) {
        return;
    }
    
    tkl_fflush(fd);
    // Get file descriptor and sync to storage
    int file_fd = tkl_fileno(fd);
    if (file_fd >= 0) {
        tkl_fsync(file_fd);
    }
}

int FS_SDCARD::feof(TUYA_FILE fd) {
    if (!ismounted || !fd) {
        return -1;
    }
    
    return tkl_feof(fd);
}

int FS_SDCARD::lseek(TUYA_FILE fd, int offs, int whence) {
    if (!ismounted || !fd) {
        return -1;
    }
    
    return tkl_fseek(fd, offs, whence);
}

int FS_SDCARD::position(TUYA_FILE fd) {
    if (!ismounted || !fd) {
        return -1;
    }
    
    return (int)tkl_ftell(fd);
}

int FS_SDCARD::filesize(const char *filepath) {
    if (!ismounted) {
        return -1;
    }
    
    String fullPath = getFullPath(filepath);
    return tkl_fgetsize(fullPath.c_str());
}
