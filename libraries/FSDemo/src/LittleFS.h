#ifndef _LITTLEFS_H_
#define _LITTLEFS_H_

#include <Arduino.h>
#include "fs_api.h"

extern "C"{
#include "lfs.h"
#include "tal_kv.h"
#include "tkl_flash.h"
#include "tal_fs.h"
#include "tal_log.h"

}

class FS_LITTLEFS : public FS_API
{
public :
	FS_LITTLEFS();
    ~FS_LITTLEFS();

    virtual int mkdir(const char *path);
    virtual int remove(const char *path);
    virtual int exist(const char *path);
    virtual int rename(const char *pathFrom,const char *pathTo);
    
	virtual TUYA_DIR openDir(const char *path);
    virtual int closeDir(TUYA_DIR dir);
    virtual TUYA_FILEINFO readDir(TUYA_DIR dir);
    virtual int getDirName(TUYA_FILEINFO info,const char** name);
    virtual int isDirectory(const char *path);


    virtual TUYA_FILE open(const char *path);
    virtual TUYA_FILE open(const char *path, const char* mode);
    virtual int  close(TUYA_FILE fd);
    virtual char read(TUYA_FILE fd);
    virtual int  read(const char *buf,int size,TUYA_FILE fd);
    virtual int  readtillN(char *buf, int size,TUYA_FILE fd);
    virtual int  write(const char *buf,int size,TUYA_FILE fd);
    virtual void flush(TUYA_FILE fd);
    virtual int  feof(TUYA_FILE fd);
    virtual int  lseek(TUYA_FILE fd,int offs, int whence);
    virtual int  position(TUYA_FILE fd);
    virtual int  filesize(const char *filepath);

    
protected:
    bool ismounted;
};



#endif
