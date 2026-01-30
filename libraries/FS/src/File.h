#ifndef __FILE_H__
#define __FILE_H__

#include "fs_api.h"
#include "Arduino.h"

enum{
	FS_NULL=0,
	LITTLEFS,
	FATFS,
	SDCARD,
};

class VFSFILE 
{
public :
	VFSFILE(int type);
	~VFSFILE();

	int mkdir(const char *path);
	int mkdir(const String& path);

	int remove(const char *path);
	int remove(const String& path);
	
	int exist(const char *path);
	int exist(const String& path);

	int rename(const char *pathFrom,const char *pathTo);
	int rename(const String& pathFrom, const String& pathTo);

	TUYA_DIR openDir(const char *path);
	TUYA_DIR openDir(const String& path);
	int closeDir(TUYA_DIR dir);
	TUYA_FILEINFO readDir(TUYA_DIR dir);
	int getDirName(TUYA_FILEINFO info,const char** name);
	int isDirectory(const char *path);

	TUYA_FILE open(const char *path);
	TUYA_FILE open(const String& path);

	TUYA_FILE open(const char *path, const char * mode);
	TUYA_FILE open(const String& path, const char * mode);

	int close(TUYA_FILE fd);
	char read(TUYA_FILE fd);
	int  read(const char *buf,int size,TUYA_FILE fd);
	int  readtillN(char *buf, int size,TUYA_FILE fd) ;
	int  write(const char *buf,int size,TUYA_FILE fd);
	void flush(TUYA_FILE fd);
	int  feof(TUYA_FILE fd);
	int  lseek(TUYA_FILE fd,int offs, int whence);
	int  position(TUYA_FILE fd);
	int  filesize(const char *filepath);

private : 
	FS_API *impl;
};

#endif

